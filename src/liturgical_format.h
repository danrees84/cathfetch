#ifndef LITURGICAL_FORMAT_H
#define LITURGICAL_FORMAT_H

#include <string>
#include "temporal_calendar.h"
#include "liturgical_colour.h"
#include <chrono>

std::string liturgicalPeriodName(LiturgicalPeriod period);
std::string liturgicalColourName(LiturgicalColour colour);
std::string formatCalendarDate(std::chrono::year_month_day date);
std::string formatOrdinal(int number);
std::string formatOrdinaryTimeDay(
    std::chrono::year_month_day date, int week);
std::string formatAdventDay(
    std::chrono::year_month_day date,
    int week);

#endif
