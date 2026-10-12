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


	std::cout << formatCalendarDate(date) << "\n";

	if (showCross) {
		std::cout << "      ███████████      ";
	}


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

	bool ordinaryTime = (period == LiturgicalPeriod::OrdinaryTime);

	if (ordinaryTime)
	{
		int week = calculateOrdinaryTimeWeek(date);

		if (showCross)
		{
			std::cout << "          ███          ";
		}

		std::cout << formatOrdinaryTimeDay(date, week) << "\n";
	}

	if (showCross)
	{
		int remainingRows = ordinaryTime ? 4 : 5;

		for (int i = 0; i < remainingRows; ++i)
		{
			std::cout << "          ███\n";
		}
	}

}
