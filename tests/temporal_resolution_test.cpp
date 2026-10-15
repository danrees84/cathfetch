// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "calendar_selection.h"
#include "liturgical_resolver.h"
#include <iostream>
#include <stdexcept>

int main()
{
    using namespace std::chrono;
    const auto calendar = loadSelectedCalendar("grc");
    struct Case { year_month_day date; const char* name; int precedence; };
    const Case cases[]{
        {2026y/February/18, "Ash Wednesday", 2},
        {2026y/February/19, "Thursday after Ash Wednesday", 9},
        {2026y/February/22, "First Sunday of Lent", 2},
        {2026y/February/23, "Monday of the First Week of Lent", 9},
        {2026y/March/29, "Palm Sunday of the Passion of the Lord", 2},
        {2026y/March/30, "Monday of Holy Week", 2},
        {2026y/April/2, "Holy Thursday", 2},
        {2026y/April/3, "Good Friday", 1},
        {2026y/April/4, "Holy Saturday", 1},
        {2026y/April/5, "Easter Sunday of the Resurrection of the Lord", 1}
    };
    for (const auto& test : cases)
    {
        const auto day = resolveLiturgicalDay(test.date, calendar);
        if (day.observanceName != test.name || int(day.precedence) != test.precedence)
        {
            std::cerr << test.date << ": " << day.observanceName << '\n';
            return 1;
        }
    }
    // A level-2 intruder must not disappear behind a named temporal baseline.
    auto conflicting = calendar;
    FixedCelebration intruder;
    intruder.id = "conflicting-highest-day";
    intruder.name = "Conflicting highest day";
    intruder.date = May/14;
    intruder.rank = CelebrationRank::Solemnity;
    intruder.precedence = PrecedenceLevel::HighestDays;
    conflicting.push_back(intruder);
    const auto unresolved = resolveLiturgicalDay(2026y/May/14, conflicting);
    if (unresolved.unresolvedReason.empty() || !unresolved.observanceId.empty() ||
        unresolved.conflictingObservances.size() != 2) return 1;
    std::cout << "Temporal resolution tests passed\n";
}
