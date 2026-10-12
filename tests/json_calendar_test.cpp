#include <iostream>
#include "calendar_loader.h"
#include "calendar_lookup.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include "calendar_overlay.h"
#include "calendar_selection.h"


int main()
{
//    auto celebrations = loadFixedCelebrations(
//        "data/calendars/general_roman_fixed.json");

auto celebrations = loadSelectedCalendar("grc");

    std::cout << "Loaded celebrations: "
              << celebrations.size() << "\n";

    for (const auto& celebration : celebrations)
    {
        if (!celebration.rank.has_value())
        {
            std::cout << "Special rank: "
                      << celebration.name << "\n";
        }
    }


auto matches = findFixedCelebrations(
    celebrations,
    std::chrono::July / std::chrono::day{11});

for (const auto& celebration : matches)
{
    std::cout << celebration.name << ": "
              << celebration.rank.value_or("special") << "\n";
}

//auto europeanCelebrations = celebrations;

//applyRankOverrides(europeanCelebrations, "data/calendars/europe.json");

auto europeanCelebrations = loadSelectedCalendar("europe");


auto europeanMatches = findFixedCelebrations(
    europeanCelebrations,
    std::chrono::July / std::chrono::day{11});

for (const auto& celebration : europeanMatches)
{
    std::cout << "Europe: " << celebration.name << ": "
              << celebration.rank.value_or("special") << "\n";
}

auto englandWalesCelebrations = europeanCelebrations;

applyCalendarOverlay(
    englandWalesCelebrations,
    "data/calendars/england_wales.json");

auto october10 = findFixedCelebrations(
    englandWalesCelebrations,
    std::chrono::October / std::chrono::day{10});

for (const auto& celebration : october10)
{
    std::cout << "England & Wales, 10 October: "
              << celebration.name << "\n";
}


auto stDavidMatches = findFixedCelebrations(
    englandWalesCelebrations,
    std::chrono::March / std::chrono::day{1});

for (const auto& celebration : stDavidMatches)
{
    std::cout << "England & Wales: "
              << celebration.name << ": "
              << celebration.rank.value_or("special") << "\n";
}

auto welshCelebrations = englandWalesCelebrations;

applyAdditions(
    welshCelebrations,
    "data/calendars/wales.json");

applyRankOverrides(
    welshCelebrations,
    "data/calendars/wales.json");


auto welshMatches = findFixedCelebrations(
    welshCelebrations,
    std::chrono::March / std::chrono::day{1});

for (const auto& celebration : welshMatches)
{
    std::cout << "Wales: "
              << celebration.name << ": "
              << celebration.rank.value_or("special") << "\n";
}


auto englishCalendar = loadSelectedCalendar("england");
auto welshCalendar = loadSelectedCalendar("wales");

for (const auto& celebration : findFixedCelebrations(
        englishCalendar, std::chrono::March / std::chrono::day{1}))
{
    std::cout << "England: " << celebration.name << " — "
              << celebration.rank.value_or("special") << "\n";
}

for (const auto& celebration : findFixedCelebrations(
        welshCalendar, std::chrono::March / std::chrono::day{1}))
{
    std::cout << "Wales (selected): " << celebration.name << " — "
              << celebration.rank.value_or("special") << "\n";
}

    return 0;
}
