// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "temporal_calendar.h"
#include "easter.h"

#include <chrono>
#include <stdexcept>

namespace
{
// year_month_day holds a readable date. sys_days represents a day on a timeline,
// so it is the type we use when adding or subtracting a number of days.
std::chrono::year_month_day daysFromEaster(int year, int offset)
{
    const std::chrono::sys_days easter{calculateEaster(year)};
    return std::chrono::year_month_day{easter + std::chrono::days{offset}};
}

std::chrono::sys_days sundayOfWeek(std::chrono::sys_days date)
{
    // c_encoding() numbers Sunday as 0, Monday as 1, and so on.
    const std::chrono::weekday weekday{date};
    return date - std::chrono::days{int(weekday.c_encoding())};
}
} // namespace

std::chrono::year_month_day calculateAshWednesday(int year)
{
    // Lent's forty fasting days do not include its six Sundays: 40 + 6 = 46.
    return daysFromEaster(year, -46);
}

std::chrono::year_month_day calculateGoodFriday(int year)
{
    return daysFromEaster(year, -2);
}

std::chrono::year_month_day calculatePalmSunday(int year)
{
    return daysFromEaster(year, -7);
}

std::chrono::year_month_day calculatePentecost(int year)
{
    // Counting Easter itself as day one makes Pentecost the fiftieth day.
    return daysFromEaster(year, 49);
}

std::chrono::year_month_day calculateAscension(int year)
{
    return daysFromEaster(year, 39);
}

std::chrono::year_month_day calculateFirstAdventSunday(int year)
{
    // The first Sunday of Advent is the Sunday on or after 27 November.
    const std::chrono::sys_days november27{
        std::chrono::year{year} / std::chrono::November / 27};
    const std::chrono::weekday weekday{november27};
    const int daysUntilSunday = (7 - weekday.c_encoding()) % 7;
    return std::chrono::year_month_day{november27 + std::chrono::days{daysUntilSunday}};
}

std::chrono::year_month_day calculateHolyFamily(int year)
{
    const std::chrono::sys_days christmas{
        std::chrono::year{year} / std::chrono::December / 25};
    const std::chrono::weekday weekday{christmas};
    if (weekday == std::chrono::Sunday)
    {
        return std::chrono::year{year} / std::chrono::December / 30;
    }

    // Otherwise use the Sunday after Christmas, not Christmas Day itself.
    const int daysUntilSunday = 7 - weekday.c_encoding();
    return std::chrono::year_month_day{christmas + std::chrono::days{daysUntilSunday}};
}

std::chrono::year_month_day calculateBaptismOfLord(int year, std::string_view calendarName)
{
    const auto epiphanyDate = calculateEpiphany(year, calendarName);
    const std::chrono::sys_days epiphany{epiphanyDate};
    const std::chrono::weekday weekday{epiphany};
    if (weekday == std::chrono::Sunday && unsigned(epiphanyDate.day()) >= 7)
    {
        // A late Sunday Epiphany leaves the Baptism on the following Monday.
        return std::chrono::year_month_day{epiphany + std::chrono::days{1}};
    }

    const int daysUntilSunday = 7 - weekday.c_encoding();
    return std::chrono::year_month_day{epiphany + std::chrono::days{daysUntilSunday}};
}

std::chrono::year_month_day calculateEpiphany(int year, std::string_view calendarName)
{
    const auto january6 = std::chrono::year{year} / std::chrono::January / 6;
    if (calendarName == "grc" || calendarName == "europe")
    {
        return january6;
    }
    if (calendarName != "england" && calendarName != "wales")
    {
        throw std::invalid_argument("Unknown liturgical calendar");
    }

    // England and Wales move a Saturday or Monday holyday to the adjacent Sunday.
    const std::chrono::sys_days date{january6};
    const std::chrono::weekday weekday{date};
    if (weekday == std::chrono::Saturday)
    {
        return std::chrono::year_month_day{date + std::chrono::days{1}};
    }
    if (weekday == std::chrono::Monday)
    {
        return std::chrono::year_month_day{date - std::chrono::days{1}};
    }
    return january6;
}

bool isAdvent(std::chrono::year_month_day date)
{
    const auto firstSunday = calculateFirstAdventSunday(int(date.year()));
    const auto christmas = date.year() / std::chrono::December / 25;
    return date >= firstSunday && date < christmas;
}

bool isChristmas(std::chrono::year_month_day date, std::string_view calendarName)
{
    const auto christmas = date.year() / std::chrono::December / 25;
    const auto baptism = calculateBaptismOfLord(int(date.year()), calendarName);
    // This season crosses New Year: either late December OR early January.
    return date >= christmas || date <= baptism;
}

bool isLent(std::chrono::year_month_day date)
{
    const auto ashWednesday = calculateAshWednesday(int(date.year()));
    const auto goodFriday = calculateGoodFriday(int(date.year()));
    return date >= ashWednesday && date < goodFriday;
}

// Date-only convention: Thursday remains Lent; Friday and Saturday are Triduum.
// Easter Sunday belongs to Easter Time and retains Triduum precedence.
// Evening transitions require a time-aware layer (UNLYC 19, 22, 28).
bool isTriduum(std::chrono::year_month_day date)
{
    const auto easter = calculateEaster(int(date.year()));
    const auto goodFriday = calculateGoodFriday(int(date.year()));
    return date >= goodFriday && date <= easter;
}

bool isEaster(std::chrono::year_month_day date)
{
    const auto easter = calculateEaster(int(date.year()));
    const auto pentecost = calculatePentecost(int(date.year()));
    return date >= easter && date <= pentecost;
}

bool isOrdinaryTime(std::chrono::year_month_day date, std::string_view calendarName)
{
    return !isAdvent(date) && !isChristmas(date, calendarName) &&
           !isLent(date) && !isTriduum(date) && !isEaster(date);
}

LiturgicalPeriod getLiturgicalPeriod(std::chrono::year_month_day date, std::string_view calendarName)
{
    if (isAdvent(date)) return LiturgicalPeriod::Advent;
    if (isChristmas(date, calendarName)) return LiturgicalPeriod::Christmas;
    if (isLent(date)) return LiturgicalPeriod::Lent;
    // Check Easter before Triduum because Easter Sunday belongs to both.
    if (isEaster(date)) return LiturgicalPeriod::Easter;
    if (isTriduum(date)) return LiturgicalPeriod::Triduum;
    return LiturgicalPeriod::OrdinaryTime;
}

int calculateOrdinaryTimeWeek(std::chrono::year_month_day date, std::string_view calendarName)
{
    if (!date.ok() || !isOrdinaryTime(date, calendarName))
    {
        throw std::invalid_argument("Date is not in Ordinary Time");
    }

    const auto weekSunday = sundayOfWeek(std::chrono::sys_days{date});
    if (date < calculateAshWednesday(int(date.year())))
    {
        // The first stretch counts forwards from the week of the Baptism.
        const auto baptism = calculateBaptismOfLord(int(date.year()), calendarName);
        const auto baptismWeekSunday = sundayOfWeek(std::chrono::sys_days{baptism});
        const int weeksSinceBaptism = int((weekSunday - baptismWeekSunday).count() / 7);
        return weeksSinceBaptism + 1;
    }

    // After Pentecost, count backwards from Advent. This keeps the final week
    // at 34 even in years when a week was skipped as Ordinary Time resumed.
    const std::chrono::sys_days advent{calculateFirstAdventSunday(int(date.year()))};
    const int weeksUntilAdvent = int((advent - weekSunday).count() / 7);
    return 35 - weeksUntilAdvent;
}

int calculateAdventWeek(std::chrono::year_month_day date)
{
    if (!date.ok() || !isAdvent(date))
    {
        throw std::invalid_argument("Date is not in Advent");
    }

    const std::chrono::sys_days firstSunday{calculateFirstAdventSunday(int(date.year()))};
    const std::chrono::sys_days calendarDay{date};
    const int daysSinceAdvent = int((calendarDay - firstSunday).count());
    return daysSinceAdvent / 7 + 1;
}
