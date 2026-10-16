// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "calendar_lookup.h"
#include "calendar_selection.h"
#include "liturgical_format.h"
#include "liturgical_precedence.h"
#include "temporal_calendar.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
void require(bool condition, const std::string &message)
{
    if (!condition)
        throw std::runtime_error(message);
}
} // namespace

int main()
{
    try
    {
        using namespace std::chrono;
        using enum PrecedenceLevel;
        require(outranks(SundayOfChristmasOrOrdinaryTime, OptionalMemorial) &&
                    !outranks(OptionalMemorial, SundayOfChristmasOrOrdinaryTime) &&
                    outranks(ProperSolemnity, OtherWeekday),
                "Strict precedence comparison");
        require(ordinaryTimeBaselinePrecedence(2026y / October / 11) == SundayOfChristmasOrOrdinaryTime &&
                    ordinaryTimeBaselinePrecedence(2025y / March / 1) == OtherWeekday,
                "Ordinary Time baseline");

        // Real definitions exercise general/proper ranks and Lord/saint feasts.
        struct FixedCase
        {
            std::string_view calendar;
            month_day date;
            PrecedenceLevel expected;
        };
        const FixedCase fixed[]{
            {"grc", October / 11, OptionalMemorial}, {"wales", March / 1, ProperSolemnity},
            {"grc", August / 6, GeneralFeastOfLord}, {"grc", August / 10, GeneralFeast},
            {"europe", July / 11, ProperFeast},      {"grc", November / 9, GeneralFeastOfLord},
            {"wales", August / 26, OptionalMemorial}};
        for (const auto &test : fixed)
        {
            const auto matches = findFixedCelebrations(loadSelectedCalendar(test.calendar), test.date);
            require(matches.size() == 1, "Expected one fixed celebration");
            require(fixedCelebrationPrecedence(matches.front()) == test.expected,
                    "Wrong fixed precedence for " + matches.front().name);
            if (test.date == October / 11)
                require(matches.front().name == "St John XXIII, Pope", "St John XXIII lookup");
            if (test.date == August / 6 || test.date == August / 10)
                require(matches.front().feastOfLord == (test.date == August / 6),
                        "Lord/saint feast classification");
        }
        require(outranks(SundayOfChristmasOrOrdinaryTime, OptionalMemorial) &&
                    outranks(OptionalMemorial, ordinaryTimeBaselinePrecedence(2025y / October / 11)),
                "Sunday excludes an optional memorial; a weekday permits it");
        require(outranks(ProperSolemnity, ordinaryTimeBaselinePrecedence(2025y / March / 1)) &&
                    outranks(OptionalMemorial, ordinaryTimeBaselinePrecedence(2026y / August / 26)),
                "Welsh celebrations outrank ordinary weekdays");
        require(outranks(GeneralFeastOfLord, ordinaryTimeBaselinePrecedence(2025y / November / 9)) &&
                    outranks(ordinaryTimeBaselinePrecedence(2025y / August / 10), GeneralFeast),
                "A feast of the Lord, but not a saint's feast, replaces an ordinary Sunday");

        struct TemporalCase
        {
            year_month_day date;
            PrecedenceLevel expected;
        };
        const TemporalCase temporal[]{{2026y / November / 29, HighestDays},
                                      {2026y / November / 30, OtherWeekday},
                                      {2026y / December / 17, PrivilegedWeekday},
                                      {2026y / December / 20, HighestDays},
                                      {2026y / February / 18, HighestDays},
                                      {2026y / February / 22, HighestDays},
                                      {2026y / February / 23, PrivilegedWeekday},
                                      {2026y / March / 30, HighestDays},
                                      {2026y / April / 2, HighestDays},
                                      {2026y / April / 3, Triduum},
                                      {2026y / April / 4, Triduum},
                                      {2026y / April / 5, Triduum},
                                      {2026y / April / 6, HighestDays},
                                      {2026y / April / 11, HighestDays},
                                      {2026y / April / 12, HighestDays},
                                      {2026y / April / 13, OtherWeekday},
                                      {2026y / May / 13, OtherWeekday},
                                      {2026y / May / 25, OtherWeekday},
                                      {2026y / December / 25, HighestDays},
                                      {2026y / December / 26, PrivilegedWeekday},
                                      {2026y / December / 27, SundayOfChristmasOrOrdinaryTime},
                                      {2027y / January / 1, PrivilegedWeekday},
                                      {2027y / January / 2, OtherWeekday},
                                      {2027y / January / 3, SundayOfChristmasOrOrdinaryTime}};
        for (const auto &test : temporal)
            require(temporalBaselinePrecedence(test.date) == test.expected,
                    "Temporal precedence: " + formatCalendarDate(test.date));

        const year_month_day ascensions[]{2025y / May / 29, 2026y / May / 14, 2027y / May / 6};
        for (auto date : ascensions)
            require(calculateAscension(int(date.year())) == date &&
                        temporalBaselinePrecedence(date) == HighestDays,
                    "Ascension date and precedence");
        require(calculatePentecost(2026) == 2026y / May / 24 &&
                    temporalBaselinePrecedence(2026y / May / 24) == HighestDays,
                "Pentecost date and precedence");
        const year_month_day families[]{2022y / December / 30, 2025y / December / 28, 2026y / December / 27};
        for (auto date : families)
            require(calculateHolyFamily(int(date.year())) == date, "Holy Family date");

        struct ScheduledCase
        {
            int year;
            std::string_view calendar;
            year_month_day date;
        };
        const ScheduledCase epiphany[]{
            {2024, "grc", 2024y / January / 6},   {2024, "england", 2024y / January / 7},
            {2025, "grc", 2025y / January / 6},   {2025, "england", 2025y / January / 5},
            {2026, "wales", 2026y / January / 6}, {2026, "europe", 2026y / January / 6}};
        for (const auto &test : epiphany)
            require(calculateEpiphany(test.year, test.calendar) == test.date, "Epiphany scheduling");
        const ScheduledCase baptism[]{{2024, "grc", 2024y / January / 7},
                                      {2024, "england", 2024y / January / 8},
                                      {2025, "england", 2025y / January / 12},
                                      {2026, "wales", 2026y / January / 11},
                                      {2030, "grc", 2030y / January / 13}};
        for (const auto &test : baptism)
            require(calculateBaptismOfLord(test.year, test.calendar) == test.date, "Baptism scheduling");
        for (const auto calendar : {"grc", "england", "wales"})
        {
            const bool roman = std::string_view{calendar} == "grc";
            require(isChristmas(2024y / January / 8, calendar) == !roman &&
                        isOrdinaryTime(2024y / January / 8, calendar) == roman &&
                        getLiturgicalPeriod(2024y / January / 8, calendar) ==
                            (roman ? LiturgicalPeriod::OrdinaryTime : LiturgicalPeriod::Christmas),
                    "Christmas boundary for " + std::string{calendar});
        }
        struct WeekCase
        {
            year_month_day date;
            std::string_view calendar;
            int week;
        };
        const WeekCase weeks[]{{2024y / January / 8, "grc", 1},      {2024y / January / 9, "england", 1},
                               {2024y / January / 14, "england", 2}, {2024y / January / 15, "wales", 2},
                               {2025y / January / 13, "england", 1}, {2025y / January / 19, "england", 2}};
        for (const auto &test : weeks)
            require(calculateOrdinaryTimeWeek(test.date, test.calendar) == test.week, "Ordinary Time week");

        // These direct formatting examples used to have their own executable.
        struct NameCase
        {
            year_month_day date;
            std::string_view name;
        };
        const NameCase advent[]{{2026y / November / 29, "First Sunday of Advent"},
                                {2026y / November / 30, "Monday of the First Week of Advent"},
                                {2026y / December / 6, "Second Sunday of Advent"},
                                {2026y / December / 17, "17 December"},
                                {2026y / December / 20, "Fourth Sunday of Advent"}};
        for (const auto &test : advent)
            require(formatAdventDay(test.date, calculateAdventWeek(test.date)) == test.name,
                    "Advent name: " + formatCalendarDate(test.date));
        std::cout << "Precedence, movable-date and Advent formatting tests passed\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
