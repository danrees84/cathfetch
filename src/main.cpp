#include <iostream>
#include <chrono>
#include <ctime>
#include <stdexcept>
// #include "easter.h"
#include "temporal_calendar.h"
#include "liturgical_format.h"
#include "liturgical_colour.h"
#include "terminal_format.h"

std::chrono::year_month_day getCurrentLocalDate()
{

	std::time_t now = std::time(nullptr);
	std::tm* local = std::localtime(&now);

	if (local == nullptr)
	{
		throw std::runtime_error("Could not retrieve local date");
	}

	std::chrono::year_month_day localDate{
		std::chrono::year{local->tm_year + 1900},
			std::chrono::month{unsigned(local->tm_mon + 1)},
			std::chrono::day{unsigned(local->tm_mday)}
	};

	return localDate;

}

int main()
{

	std::chrono::year_month_day todayDate = getCurrentLocalDate();
	std::cout << "Horarium\n\n";
	std::cout << todayDate.day() << "-" << todayDate.month() << "-" << todayDate.year() << "\n";

	auto period = getLiturgicalPeriod(todayDate);
	auto colour = seasonalColour(period);

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

