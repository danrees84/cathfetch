#include "liturgical_format.h"
#include "liturgical_colour.h"
#include <format>

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

    int lastTwo = number % 100;

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
