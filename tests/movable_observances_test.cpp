// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "movable_observances.h"

#include <algorithm>
#include <iostream>

int main()
{
    using namespace std::chrono;
    using enum PrecedenceLevel;
    struct Case
    {
        year_month_day date;
        std::string_view calendar;
        std::string_view id;
        std::string_view name;
        CelebrationRank rank;
        PrecedenceLevel precedence;
        bool namesBaseline;
    };
    const Case cases[]{
        {2026y / December / 27, "grc", "holy_family", "The Holy Family of Jesus, Mary and Joseph",
         CelebrationRank::Feast, GeneralFeastOfLord, false},
        {2022y / December / 30, "grc", "holy_family", "The Holy Family of Jesus, Mary and Joseph",
         CelebrationRank::Feast, GeneralFeastOfLord, false},
        {2024y / January / 7, "grc", "baptism_of_lord", "The Baptism of the Lord", CelebrationRank::Feast,
         GeneralFeastOfLord, false},
        {2024y / January / 8, "england", "baptism_of_lord", "The Baptism of the Lord", CelebrationRank::Feast,
         GeneralFeastOfLord, false},
        {2026y / January / 11, "wales", "baptism_of_lord", "The Baptism of the Lord", CelebrationRank::Feast,
         GeneralFeastOfLord, false},
        {2026y / May / 14, "grc", "ascension", "The Ascension of the Lord", CelebrationRank::Solemnity,
         HighestDays, true},
        {2026y / May / 24, "grc", "pentecost", "Pentecost Sunday", CelebrationRank::Solemnity, HighestDays,
         true},
        {2026y / April / 5, "grc", "easter_sunday", "Easter Sunday of the Resurrection of the Lord",
         CelebrationRank::Solemnity, Triduum, true}};
    for (const auto &test : cases)
    {
        const auto candidates = findMovableObservances(test.date, test.calendar);
        const auto found = std::ranges::find(candidates, test.id, &ObservanceCandidate::id);
        if (candidates.size() != 1 || found == candidates.end() || found->name != test.name ||
            found->rank != test.rank || found->precedence != test.precedence ||
            found->namesTemporalBaseline != test.namesBaseline)
        {
            std::cerr << "Wrong movable candidate: " << test.date << " (" << test.calendar << ")\n";
            return 1;
        }
    }
    if (!findMovableObservances(2026y / December / 26).empty() ||
        !findMovableObservances(2024y / January / 7, "england").empty())
    {
        std::cerr << "Unexpected candidate on St Stephen or English Epiphany\n";
        return 1;
    }
    std::cout << "Movable candidate tests passed\n";
}
