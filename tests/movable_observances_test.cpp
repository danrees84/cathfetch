// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include <chrono>
#include <iostream>

#include "movable_observances.h"

int main()
{
    auto holyFamily2026 = findMovableObservances(
        std::chrono::year{2026} / std::chrono::December / 27);

    if (holyFamily2026.size() != 1 ||
        holyFamily2026.front().id != "holy_family" ||
        holyFamily2026.front().precedence !=
            PrecedenceLevel::GeneralFeastOfLord)
    {
        std::cerr << "Holy Family 2026 test failed\n";
        return 1;
    }

    auto holyFamily2022 = findMovableObservances(
        std::chrono::year{2022} / std::chrono::December / 30);

    if (holyFamily2022.size() != 1 ||
        holyFamily2022.front().id != "holy_family")
    {
        std::cerr << "Holy Family 2022 test failed\n";
        return 1;
    }

    auto december26 = findMovableObservances(
        std::chrono::year{2026} / std::chrono::December / 26);

    if (!december26.empty())
    {
        std::cerr << "Unexpected movable observance\n";
        return 1;
    }


struct BaptismTestCase
{
    std::chrono::year_month_day date;
    std::string_view calendarName;
};

const BaptismTestCase baptismTests[] = {
    {std::chrono::year{2024} / std::chrono::January / 7, "grc"},
    {std::chrono::year{2024} / std::chrono::January / 8, "england"},
    {std::chrono::year{2026} / std::chrono::January / 11, "wales"}
};

for (const auto& test : baptismTests)
{
    auto candidates = findMovableObservances(
        test.date, test.calendarName);

    if (candidates.size() != 1 ||
        candidates.front().id != "baptism_of_lord" ||
        candidates.front().precedence !=
            PrecedenceLevel::GeneralFeastOfLord)
    {
        std::cerr << "Baptism candidate test failed: "
                  << test.date << " ("
                  << test.calendarName << ")\n";
        return 1;
    }
}

auto englandJanuary7 = findMovableObservances(
    std::chrono::year{2024} / std::chrono::January / 7,
    "england");

if (!englandJanuary7.empty())
{
    std::cerr << "Baptism incorrectly generated on Epiphany\n";
    return 1;
}

auto ascensionCandidates = findMovableObservances(
    std::chrono::year{2026} / std::chrono::May / 14,
    "grc");

bool ascensionFound = false;

for (const auto& candidate : ascensionCandidates)
{
    if (candidate.id == "ascension")
    {
        ascensionFound =
            candidate.name == "The Ascension of the Lord" &&
            candidate.rank == CelebrationRank::Solemnity &&
            candidate.precedence == PrecedenceLevel::HighestDays;
    }
}

if (!ascensionFound)
{
    std::cerr << "Ascension movable observance test failed\n";
    return 1;
}

auto pentecostCandidates = findMovableObservances(
    std::chrono::year{2026} / std::chrono::May / 24,
    "grc");

bool pentecostFound = false;

for (const auto& candidate : pentecostCandidates)
{
    if (candidate.id == "pentecost")
    {
        pentecostFound =
            candidate.name == "Pentecost Sunday" &&
            candidate.rank == CelebrationRank::Solemnity &&
            candidate.precedence == PrecedenceLevel::HighestDays;
    }
}

if (!pentecostFound)
{
    std::cerr << "Pentecost movable observance test failed\n";
    return 1;
}

auto easterCandidates = findMovableObservances(
    std::chrono::year{2026} / std::chrono::April / 5,
    "grc");

bool easterFound = false;

for (const auto& candidate : easterCandidates)
{
    if (candidate.id == "easter_sunday")
    {
        easterFound =
            candidate.name ==
                "Easter Sunday of the Resurrection of the Lord" &&
            candidate.rank == CelebrationRank::Solemnity &&
            candidate.precedence == PrecedenceLevel::Triduum &&
            candidate.namesTemporalBaseline;
    }
}

if (!easterFound)
{
    std::cerr << "Easter Sunday movable observance test failed\n";
    return 1;
}

    std::cout << "Movable observance tests passed\n";
    return 0;
}
