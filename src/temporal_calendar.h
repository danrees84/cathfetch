#ifndef HORARIUM_TEMPORAL_CALENDAR_H
#define HORARIUM_TEMPORAL_CALENDAR_H

#include <chrono>

std::chrono::year_month_day calculateAshWednesday(int year);
std::chrono::year_month_day calculatePalmSunday(int year);
std::chrono::year_month_day calculatePentecost(int year);
std::chrono::year_month_day calculateFirstAdventSunday(int year);

#endif
