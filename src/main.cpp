#include <iostream>
#include <chrono>
#include <ctime>
#include <stdexcept>
#include "easter.h"
#include "temporal_calendar.h"

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
	auto easterDate = calculateEaster(int(todayDate.year()));
	std::cout << "Easter date: " << unsigned(easterDate.day()) << "-" << unsigned(easterDate.month()) << "-" << int(easterDate.year()) << "\n";

	auto ashWednesdayDate = calculateAshWednesday(int(todayDate.year()));
	std::cout << "Ash Wednesday date: " << unsigned(ashWednesdayDate.day()) << "-" << unsigned(ashWednesdayDate.month()) << "-" << int(ashWednesdayDate.year()) << "\n";

	auto palmSundayDate = calculatePalmSunday(int(todayDate.year()));
	std::cout << "Palm Sunday date: " << unsigned(palmSundayDate.day()) << "-" << unsigned(palmSundayDate.month()) << "-" << int(palmSundayDate.year()) << "\n";

	auto pentecostDate = calculatePentecost(int(todayDate.year()));
	std::cout << "Pentecost date: " << unsigned(pentecostDate.day()) << "-" << unsigned(pentecostDate.month()) << "-" << int(pentecostDate.year()) << "\n";

	auto adventDate = calculateFirstAdventSunday(int(todayDate.year()));
	std::cout << "Advent date: " << unsigned(adventDate.day()) << "-" << unsigned(adventDate.month()) << "-" << int(adventDate.year()) << "\n";




}

