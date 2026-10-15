// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include <iostream>
#include "calendar_loader.h"
#include "calendar_lookup.h"
#include <nlohmann/json.hpp>
#include "calendar_overlay.h"
#include "calendar_selection.h"
#include "celebration_rank.h"

int main()
{
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
              <<  celebrationRankName(celebration.rank) << "\n";
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
              << celebrationRankName(celebration.rank)
    << " (rank from "
              << celebration.rankSourceCalendar << ")\n";
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
              << celebrationRankName(celebration.rank)  << "\n";
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
              << celebrationRankName(celebration.rank)
	       << " (rank from "
              << celebration.rankSourceCalendar << ")\n";

}


auto englishCalendar = loadSelectedCalendar("england");
auto welshCalendar = loadSelectedCalendar("wales");

for (const auto& celebration : findFixedCelebrations(
        englishCalendar, std::chrono::March / std::chrono::day{1}))
{
    std::cout << "England: " << celebration.name << " — "
              << celebrationRankName(celebration.rank) << "\n";
}

for (const auto& celebration : findFixedCelebrations(
        welshCalendar, std::chrono::March / std::chrono::day{1}))
{
    std::cout << "Wales (selected): " << celebration.name << " — "
              <<  celebrationRankName(celebration.rank) << "\n";
}

auto transfiguration = findFixedCelebrations(
    celebrations,
    std::chrono::August / std::chrono::day{6});

auto stLawrence = findFixedCelebrations(
    celebrations,
    std::chrono::August / std::chrono::day{10});

if (transfiguration.size() != 1 || stLawrence.size() != 1)
{
    std::cerr << "Feast lookup failed\n";
    return 1;
}

if (!transfiguration.front().feastOfLord ||
    stLawrence.front().feastOfLord)
{
    std::cerr << "Feast classification failed\n";
    return 1;
}

std::cout << "Feast classification test passed\n";

    return 0;
}
