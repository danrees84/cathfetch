// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "lectionary_cycle.h"
#include "temporal_calendar.h"
#include <stdexcept>

ReadingCycles readingCycles(std::chrono::year_month_day date)
{
    const int civilYear = int(date.year());
    if (!date.ok() || civilYear < 1583 || civilYear > 4099)
        throw std::invalid_argument("Lectionary date must be valid and within 1583–4099");
    // Name the liturgical year by the civil year in which it ends. For example,
    // Advent 2026 starts the liturgical year ending in 2027.
    int liturgicalYear = civilYear;
    if (date >= calculateFirstAdventSunday(civilYear))
    {
        ++liturgicalYear;
    }

    SundayCycle sunday = SundayCycle::C;
    if (liturgicalYear % 3 == 1) sunday = SundayCycle::A;
    else if (liturgicalYear % 3 == 2) sunday = SundayCycle::B;
    const auto weekday = liturgicalYear % 2 == 1 ? WeekdayCycle::I : WeekdayCycle::II;
    return {liturgicalYear, sunday, weekday};
}

std::string_view cycleName(SundayCycle cycle)
{
    switch (cycle)
    {
        case SundayCycle::A: return "A";
        case SundayCycle::B: return "B";
        case SundayCycle::C: return "C";
    }
    throw std::invalid_argument("Invalid Sunday reading cycle");
}

std::string_view cycleName(WeekdayCycle cycle)
{
    switch (cycle)
    {
        case WeekdayCycle::I: return "I";
        case WeekdayCycle::II: return "II";
    }
    throw std::invalid_argument("Invalid weekday reading cycle");
}
