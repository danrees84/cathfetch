// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "liturgical_format.h"
#include "liturgical_colour.h"
#include <format>
#include <array>
#include <string_view>
#include "temporal_calendar.h"
#include "easter.h"
#include <stdexcept>

std::string liturgicalPeriodName(LiturgicalPeriod period)
{
    switch (period)
    {
        case LiturgicalPeriod::Advent:
            return "Advent";

        case LiturgicalPeriod::Christmas:
            return "Christmas Time";
        case LiturgicalPeriod::Easter:
            return "Easter Time";
        case LiturgicalPeriod::Lent:
            return "Lent";
        case LiturgicalPeriod::OrdinaryTime:
            return "Ordinary Time";
        case LiturgicalPeriod::Triduum:
            return "Sacred Paschal Triduum";

    }

    return "Unknown";
}

std::string liturgicalColourName(LiturgicalColour colour)
{
    switch (colour)
    {
        case LiturgicalColour::White:
            return "White";
        case LiturgicalColour::Violet:
            return "Violet";
        case LiturgicalColour::Black:
            return "Black";
        case LiturgicalColour::Green:
            return "Green";
        case LiturgicalColour::Red:
            return "Red";
        case LiturgicalColour::Rose:
            return "Rose";
    }

    return "Unknown";
}

std::string formatCalendarDate(std::chrono::year_month_day date)
{
    return std::format(
            "{:%A}, {} {:%B} {}",
            std::chrono::sys_days{date},
            unsigned(date.day()),
            date.month(),
            int(date.year())
            );
}

std::string formatOrdinal(int number)
{
    std::string suffix = "th";

    // The teens are exceptions: 11th, 12th and 13th, rather than 11st, etc.
    const int lastTwo = number % 100;

    if (lastTwo < 11 || lastTwo > 13)
    {
        switch (number % 10)
        {
            case 1:
                suffix = "st";
                break;
            case 2:
                suffix = "nd";
                break;
            case 3:
                suffix = "rd";
                break;
        }
    }

    return std::to_string(number) + suffix;
}

std::string formatOrdinaryTimeDay(
        std::chrono::year_month_day date, int week)
{
    std::chrono::sys_days calendarDay{date};
    std::chrono::weekday dayOfWeek{calendarDay};

    if (dayOfWeek == std::chrono::Sunday)
    {
        return formatOrdinal(week) + " Sunday in Ordinary Time";
    }

    return std::format("{:%A}", calendarDay)
        + " of the " + formatOrdinal(week)
        + " Week in Ordinary Time";
}

std::string formatAdventDay(
        std::chrono::year_month_day date,
        int week)
{
    constexpr std::array<std::string_view, 4> weekNames{
        "First",
        "Second",
        "Third",
        "Fourth"
    };

    std::string_view weekName = weekNames.at(week - 1);

    std::chrono::sys_days calendarDay{date};
    std::chrono::weekday dayOfWeek{calendarDay};

    if (dayOfWeek == std::chrono::Sunday)
    {
        return std::format("{} Sunday of Advent", weekName);
    }

    if (date.month() == std::chrono::December &&
            unsigned(date.day()) >= 17)
    {
        return std::format("{} December", unsigned(date.day()));
    }

    return std::format(
            "{:%A} of the {} Week of Advent",
            calendarDay,
            weekName);
}

std::string formatChristmasDay(
        std::chrono::year_month_day date,
        std::string_view calendarName)
{
    std::chrono::sys_days calendarDay{date};
    std::chrono::weekday dayOfWeek{calendarDay};

    if (date.month() == std::chrono::January &&
            unsigned(date.day()) >= 2 &&
            unsigned(date.day()) <= 5 &&
            dayOfWeek == std::chrono::Sunday)
    {
        return "Second Sunday after Christmas";
    }

    if (date.month() == std::chrono::December)
    {
        switch (unsigned(date.day()))
        {
            case 29:
                return "Fifth Day within the Octave of the Nativity of the Lord";

            case 30:
                return "Sixth Day within the Octave of the Nativity of the Lord";

            case 31:
                return "Seventh Day within the Octave of the Nativity of the Lord";
        }
    }

    auto epiphanyDate = calculateEpiphany(
            int(date.year()), calendarName);

    auto baptismDate = calculateBaptismOfLord(
            int(date.year()), calendarName);

    if (date > epiphanyDate && date < baptismDate)
    {
        return std::format(
                "{:%A} after Epiphany",
                std::chrono::sys_days{date});
    }
    return std::format(
            "{} {:%B}",
            unsigned(date.day()),
            date.month()
            );
}

std::string formatEasterDay(
    std::chrono::year_month_day date)
{
    std::chrono::sys_days calendarDay{date};

    std::chrono::sys_days easterSunday{
        calculateEaster(int(date.year()))
    };

    if (calendarDay > easterSunday &&
        calendarDay < easterSunday + std::chrono::days{7})
    {
        return std::format(
            "{:%A} within the Octave of Easter",
            calendarDay);
    }

    const std::chrono::days daysSinceEaster = calendarDay - easterSunday;

    std::chrono::weekday dayOfWeek{calendarDay};

    constexpr std::array<std::string_view, 6> weekNames{
        "Second",
        "Third",
        "Fourth",
        "Fifth",
        "Sixth",
        "Seventh"
    };

    int weekNumber = int(daysSinceEaster.count() / 7) + 1;

    if (dayOfWeek == std::chrono::Sunday &&
        daysSinceEaster >= std::chrono::days{7} &&
        daysSinceEaster <= std::chrono::days{42})
    {
        std::string_view weekName =
            weekNames.at(weekNumber - 2);

        if (weekNumber == 2)
        {
            return "Second Sunday of Easter (or of Divine Mercy)";
        }

        return std::format("{} Sunday of Easter", weekName);
    }

    if (dayOfWeek != std::chrono::Sunday &&
        daysSinceEaster >= std::chrono::days{8} &&
        daysSinceEaster <= std::chrono::days{48})
    {
        std::string_view weekName =
            weekNames.at(weekNumber - 2);

        return std::format(
            "{:%A} of the {} Week of Easter",
            calendarDay,
            weekName);
    }

    throw std::logic_error(
        "Easter day formatting not yet implemented");
}

std::string formatLentenDay(std::chrono::year_month_day date)
{
    using namespace std::chrono;
    const sys_days day{date};
    const sys_days easter{calculateEaster(int(date.year()))};
    const auto offset = (day - easter).count();
    if (offset == -46) return "Ash Wednesday";
    if (offset < -42) return std::format("{:%A} after Ash Wednesday", day);
    if (offset == -7) return "Palm Sunday of the Passion of the Lord";
    if (offset == -3) return "Holy Thursday";
    if (offset == -2) return "Good Friday";
    if (offset == -1) return "Holy Saturday";
    if (offset > -7) return std::format("{:%A} of Holy Week", day);
    constexpr std::array<std::string_view, 5> weeks{
        "First", "Second", "Third", "Fourth", "Fifth"};
    const auto week = (offset + 42) / 7;
    if (weekday{day} == Sunday)
        return std::format("{} Sunday of Lent", weeks.at(week));
    return std::format("{:%A} of the {} Week of Lent", day, weeks.at(week));
}
