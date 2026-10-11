#ifndef HORARIUM_TEMPORAL_CALENDAR_H
#define HORARIUM_TEMPORAL_CALENDAR_H

#include <chrono>

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
std::chrono::year_month_day calculateFirstAdventSunday(int year);
std::chrono::year_month_day calculateBaptismOfLord(int year);
bool isAdvent(std::chrono::year_month_day date);
bool isChristmas(std::chrono::year_month_day date);
bool isLent(std::chrono::year_month_day date);
bool isTriduum(std::chrono::year_month_day date);
bool isEaster(std::chrono::year_month_day date);
bool isOrdinaryTime(std::chrono::year_month_day date);
LiturgicalPeriod getLiturgicalPeriod(std::chrono::year_month_day date);

#endif
