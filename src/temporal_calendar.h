// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#ifndef TEMPORAL_CALENDAR_H
#define TEMPORAL_CALENDAR_H

#include <chrono>
#include <string_view>

// "Temporal" means the seasonal calendar: Sundays, weekdays and the movable
// celebrations related to Easter and Christmas, rather than a saint's proper.
enum class LiturgicalPeriod
{
    Advent,
    Christmas,
    Lent,
    Easter,
    Triduum,
    OrdinaryTime
};

std::chrono::year_month_day calculateAshWednesday(int year);
std::chrono::year_month_day calculateGoodFriday(int year);
std::chrono::year_month_day calculatePalmSunday(int year);
std::chrono::year_month_day calculatePentecost(int year);
std::chrono::year_month_day calculateAscension(int year);
std::chrono::year_month_day calculateFirstAdventSunday(int year);
std::chrono::year_month_day calculateBaptismOfLord(
    int year,
    std::string_view calendarName = "grc");
std::chrono::year_month_day calculateHolyFamily(int year);
bool isAdvent(std::chrono::year_month_day date);
bool isChristmas(
    std::chrono::year_month_day date,
    std::string_view calendarName = "grc");
bool isLent(std::chrono::year_month_day date);
bool isTriduum(std::chrono::year_month_day date);
bool isEaster(std::chrono::year_month_day date);
bool isOrdinaryTime(
    std::chrono::year_month_day date,
    std::string_view calendarName = "grc");
LiturgicalPeriod getLiturgicalPeriod(
    std::chrono::year_month_day date,
    std::string_view calendarName = "grc");
int calculateOrdinaryTimeWeek(
    std::chrono::year_month_day date,
    std::string_view calendarName = "grc");
int calculateAdventWeek(std::chrono::year_month_day date);
std::chrono::year_month_day calculateEpiphany(
    int year,
    std::string_view calendarName);

#endif
