#include "terminal_display.h"
#include "temporal_calendar.h"
#include "liturgical_colour.h"
#include "liturgical_format.h"
#include "terminal_format.h"

#include <iostream>

void printLiturgicalDay(std::chrono::year_month_day date)
{
	auto period = getLiturgicalPeriod(date);
	auto colour = seasonalColour(period);

	std::cout << date.day() << "-" << date.month() << "-" << date.year() << "\n";

	if (colour.has_value())
	{
		std::cout << terminalColourCode(colour.value())
			<< "●"
			<< "\033[0m"
			<< " " << liturgicalPeriodName(period) << "\n";
	}
	else
	{
		std::cout << liturgicalPeriodName(period) << "\n";
	}



}
