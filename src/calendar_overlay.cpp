// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "calendar_overlay.h"
#include "calendar_loader.h"

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>
#include <stdexcept>
#include <utility>

namespace
{
nlohmann::json readOverlay(const std::string& filename)
{
    std::ifstream file{filename};
    if (!file)
    {
        throw std::runtime_error("Could not open calendar overlay: " + filename);
    }
    nlohmann::json overlay;
    file >> overlay;
    return overlay;
}

// The private helpers work with an already parsed document. Applying a whole
// overlay therefore reads the file once, even though it has three kinds of change.
void addCelebrations(std::vector<FixedCelebration>& celebrations, const nlohmann::json& overlay)
{
    const std::string sourceCalendar = overlay.at("calendar");
    for (const auto& entry : overlay.at("additions"))
    {
        const std::string id = entry.at("id");
        const auto existing = std::ranges::find_if(celebrations, [&](const auto& celebration) {
            return celebration.id == id;
        });
        if (existing != celebrations.end())
        {
            throw std::runtime_error("Duplicate calendar celebration ID: " + id);
        }
        celebrations.push_back(parseFixedCelebration(entry, sourceCalendar));
    }
}

void overrideCelebrations(std::vector<FixedCelebration>& celebrations, const nlohmann::json& overlay)
{
    const std::string sourceCalendar = overlay.at("calendar");
    const std::set<std::string> allowedChanges{
        "rank", "name", "feast_of_lord", "precedence", "names_temporal_baseline",
        "date_rule", "transfer_rule", "colour", "subject", "principal_patron"};
    std::set<std::string> overriddenIds;
    for (const auto& entry : overlay.at("overrides"))
    {
        const std::string id = entry.at("id");
        if (!overriddenIds.insert(id).second)
        {
            throw std::invalid_argument("Duplicate calendar override ID: " + id);
        }
        // Reject misspelt fields rather than silently ignoring the intended change.
        for (const auto& [key, value] : entry.at("changes").items())
        {
            (void)value; // Only the field names are checked here.
            if (!allowedChanges.contains(key))
            {
                throw std::invalid_argument("Unknown overlay change: " + key);
            }
        }

        auto celebration = std::ranges::find_if(celebrations, [&](const auto& value) {
            return value.id == id;
        });
        if (celebration == celebrations.end())
        {
            throw std::runtime_error("Unknown calendar override ID: " + id);
        }
        updateCelebrationMetadata(*celebration, entry.at("changes"), sourceCalendar);
    }
}

void reassignDates(std::vector<FixedCelebration>& celebrations, const nlohmann::json& overlay)
{
    std::set<std::string> reassignedIds;
    for (const auto& entry : overlay.at("date_reassignments"))
    {
        const std::string id = entry.at("id");
        if (!reassignedIds.insert(id).second)
        {
            throw std::invalid_argument("Duplicate date reassignment ID: " + id);
        }
        const int month = entry.at("new_month");
        const int day = entry.at("new_day");
        if (month < 1 || month > 12 || day < 1 || day > 31)
        {
            throw std::invalid_argument("Invalid reassignment date for: " + id);
        }
        const auto newDate = std::chrono::month{unsigned(month)} / std::chrono::day{unsigned(day)};
        if (!newDate.ok())
        {
            throw std::runtime_error("Invalid reassignment date for: " + id);
        }

        auto celebration = std::ranges::find_if(celebrations, [&](const auto& value) {
            return value.id == id;
        });
        if (celebration == celebrations.end())
        {
            throw std::runtime_error("Unknown calendar reassignment ID: " + id);
        }
        celebration->date = newDate;
    }
}
} // namespace

void applyRankOverrides(std::vector<FixedCelebration>& celebrations, const std::string& filename)
{
    overrideCelebrations(celebrations, readOverlay(filename));
}

void applyAdditions(std::vector<FixedCelebration>& celebrations, const std::string& filename)
{
    addCelebrations(celebrations, readOverlay(filename));
}

void applyDateReassignments(std::vector<FixedCelebration>& celebrations, const std::string& filename)
{
    reassignDates(celebrations, readOverlay(filename));
}

void applyCalendarOverlay(std::vector<FixedCelebration>& celebrations, const std::string& filename)
{
    const auto overlay = readOverlay(filename);
    // Work on a copy so a failed change leaves the caller's calendar intact.
    // Additions come first: later changes may refer to a newly added celebration.
    auto updated = celebrations;
    addCelebrations(updated, overlay);
    overrideCelebrations(updated, overlay);
    reassignDates(updated, overlay);
    validateCalendar(updated);
    celebrations = std::move(updated);
}
