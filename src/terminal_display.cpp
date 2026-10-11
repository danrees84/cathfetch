#include "terminal_display.h"
#include "temporal_calendar.h"
#include "liturgical_colour.h"
#include "liturgical_format.h"
#include "terminal_format.h"

#include <iostream>

void printLiturgicalDay(std::chrono::year_month_day date, bool showCross)
{
	auto period = getLiturgicalPeriod(date);
	auto colour = seasonalColour(period);

	if (showCross) {
		std::cout << "          ███\n";
		std::cout << "          ███          ";
	}
	
//	std::cout << date.day() << "-" << date.month()
//		<< "-" << date.year() << "\n";

std::cout << formatCalendarDate(date) << "\n";

	if (showCross) {
		std::cout << "      ███████████      ";
	}

	//	std::cout << date.day() << "-" << date.month() << "-" << date.year() << "\n";

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
	if (showCross) {
		for (int i = 0; i < 5; ++i)
		{
			std::cout << "          ███\n";
		}
	}


}
