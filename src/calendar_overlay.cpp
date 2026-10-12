#include "calendar_overlay.h"
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <optional>

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

    for (const auto& overrideEntry : overlay.at("overrides"))
    {
        std::string id =
            overrideEntry.at("id").get<std::string>();

        std::string newRank =
            overrideEntry.at("changes").at("rank").get<std::string>();

        bool found = false;

        for (auto& celebration : celebrations)
        {
            if (celebration.id == id)
            {
                celebration.rank = newRank;
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

        int month = entry.at("month").get<int>();
        int day = entry.at("day").get<int>();

        std::chrono::month_day date{
            std::chrono::month{unsigned(month)} /
            std::chrono::day{unsigned(day)}
        };

        if (!date.ok())
        {
            throw std::runtime_error(
                "Invalid calendar addition date: " + id);
        }

        std::optional<std::string> rank;

        if (!entry.at("rank").is_null())
        {
            rank = entry.at("rank").get<std::string>();
        }

        FixedCelebration celebration{
            id,
            date,
            entry.at("name").get<std::string>(),
            rank
        };

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

    for (const auto& reassignment : overlay.at("date_reassignments"))
    {
        std::string id = reassignment.at("id").get<std::string>();

        int month = reassignment.at("new_month").get<int>();
        int day = reassignment.at("new_day").get<int>();

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
    applyAdditions(celebrations, filename);
    applyRankOverrides(celebrations, filename);
    applyDateReassignments(celebrations, filename);
}
