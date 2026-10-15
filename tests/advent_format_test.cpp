// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include <chrono>
#include <iostream>
#include <string_view>

#include "temporal_calendar.h"
#include "liturgical_format.h"

int main()
{
    struct TestCase
    {
        std::chrono::year_month_day date;
        std::string_view expected;
    };

    const TestCase tests[] = {
        {std::chrono::year{2026} / std::chrono::November / 29,
         "First Sunday of Advent"},

        {std::chrono::year{2026} / std::chrono::November / 30,
         "Monday of the First Week of Advent"},

        {std::chrono::year{2026} / std::chrono::December / 6,
         "Second Sunday of Advent"},

        {std::chrono::year{2026} / std::chrono::December / 17,
         "17 December"},

        {std::chrono::year{2026} / std::chrono::December / 20,
         "Fourth Sunday of Advent"}
    };

    for (const auto& test : tests)
    {
        int week = calculateAdventWeek(test.date);
        auto actual = formatAdventDay(test.date, week);

        if (actual != test.expected)
        {
            std::cerr << "Expected: " << test.expected << "\n";
            std::cerr << "Actual: " << actual << "\n";
            return 1;
        }
    }

    std::cout << "Advent formatting tests passed\n";
    return 0;
}
