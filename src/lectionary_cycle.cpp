#include "lectionary_cycle.h"
#include "temporal_calendar.h"
#include <stdexcept>

ReadingCycles readingCycles(std::chrono::year_month_day date)
{
    const int civilYear = int(date.year());
    if (!date.ok() || civilYear < 1583 || civilYear > 4099)
        throw std::invalid_argument("Lectionary date must be valid and within 1583–4099");
    const int year = civilYear + (date >= calculateFirstAdventSunday(civilYear) ? 1 : 0);
    const auto sunday = year % 3 == 1 ? SundayCycle::A :
                        year % 3 == 2 ? SundayCycle::B : SundayCycle::C;
    return {year, sunday, year % 2 ? WeekdayCycle::I : WeekdayCycle::II};
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
