#include "calendar_overlay.h"
#include "calendar_loader.h"
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include "celebration_rank.h"

void applyRankOverrides(
    std::vector<FixedCelebration>& celebrations,
    const std::string& filename)
{
    std::ifstream overlayFile{filename};

    if (!overlayFile)
    {
        throw std::runtime_error(
            "Could not open calendar overlay: " + filename);
    }

    nlohmann::json overlay;
    overlayFile >> overlay;

std::string sourceCalendar =
    overlay.at("calendar").get<std::string>();

    std::set<std::string> overriddenIds;
    for (const auto& overrideEntry : overlay.at("overrides"))
    {
        std::string id =
            overrideEntry.at("id").get<std::string>();
        if (!overriddenIds.insert(id).second)
            throw std::invalid_argument("Duplicate calendar override ID: " + id);
        const std::set<std::string> allowedChanges{
            "rank", "name", "feast_of_lord", "precedence", "names_temporal_baseline",
            "date_rule", "transfer_rule", "colour", "subject", "principal_patron"};
        for (const auto& [key, value] : overrideEntry.at("changes").items())
        {
            (void)value;
            if (!allowedChanges.contains(key)) throw std::invalid_argument("Unknown overlay change: " + key);
        }



	bool found = false;

        for (auto& celebration : celebrations)
        {
            if (celebration.id == id)
            {
                updateCelebrationMetadata(celebration, overrideEntry.at("changes"), sourceCalendar);
                found = true;
                break;
            }
        }

        if (!found)
        {
            throw std::runtime_error(
                "Unknown calendar override ID: " + id);
        }
    }
}

void applyAdditions(
    std::vector<FixedCelebration>& celebrations,
    const std::string& filename)
{
    std::ifstream overlayFile{filename};

    if (!overlayFile)
    {
        throw std::runtime_error(
            "Could not open calendar overlay: " + filename);
    }

    nlohmann::json overlay;
    overlayFile >> overlay;

    std::string sourceCalendar =
    overlay.at("calendar").get<std::string>();

    for (const auto& entry : overlay.at("additions"))
    {
        std::string id = entry.at("id").get<std::string>();

        for (const auto& celebration : celebrations)
        {
            if (celebration.id == id)
            {
                throw std::runtime_error(
                    "Duplicate calendar celebration ID: " + id);
            }
        }

        auto celebration = parseFixedCelebration(entry, sourceCalendar);

        celebrations.push_back(celebration);
    }
}

void applyDateReassignments(
    std::vector<FixedCelebration>& celebrations,
    const std::string& filename)
{
    std::ifstream overlayFile{filename};

    if (!overlayFile)
    {
        throw std::runtime_error(
            "Could not open calendar overlay: " + filename);
    }

    nlohmann::json overlay;
    overlayFile >> overlay;

    std::set<std::string> reassignedIds;
    for (const auto& reassignment : overlay.at("date_reassignments"))
    {
        std::string id = reassignment.at("id").get<std::string>();
        if (!reassignedIds.insert(id).second)
            throw std::invalid_argument("Duplicate date reassignment ID: " + id);

        int month = reassignment.at("new_month").get<int>();
        int day = reassignment.at("new_day").get<int>();

        if (month < 1 || month > 12 || day < 1 || day > 31)
            throw std::invalid_argument("Invalid reassignment date for: " + id);
        std::chrono::month_day newDate{
            std::chrono::month{unsigned(month)} /
            std::chrono::day{unsigned(day)}
        };

        if (!newDate.ok())
        {
            throw std::runtime_error(
                "Invalid reassignment date for: " + id);
        }

        bool found = false;

        for (auto& celebration : celebrations)
        {
            if (celebration.id == id)
            {
                celebration.date = newDate;
                found = true;
                break;
            }
        }

        if (!found)
        {
            throw std::runtime_error(
                "Unknown calendar reassignment ID: " + id);
        }
    }
}

void applyCalendarOverlay(
    std::vector<FixedCelebration>& celebrations,
    const std::string& filename)
{
    auto updated = celebrations;
    applyAdditions(updated, filename);
    applyRankOverrides(updated, filename);
    applyDateReassignments(updated, filename);
    validateCalendar(updated);
    celebrations = std::move(updated);
}
