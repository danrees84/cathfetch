#include <chrono>
#include "temporal_calendar.h"
#include "easter.h"
// #include <iostream>

std::chrono::year_month_day calculateAshWednesday(int year)
{

	auto easterDate = calculateEaster(year);
	std::chrono::sys_days easterDay{easterDate};

	auto ashWednesdayDay = easterDay - std::chrono::days{46};

	std::chrono::year_month_day ashWednesdayDate{ashWednesdayDay};

	return ashWednesdayDate;

}

std::chrono::year_month_day calculatePalmSunday(int year)
{

	auto easterDate = calculateEaster(year);
	std::chrono::sys_days easterDay{easterDate};

	auto palmSundayDay = easterDay - std::chrono::days{7};

	std::chrono::year_month_day palmSundayDate{palmSundayDay};

	return palmSundayDate;

}

std::chrono::year_month_day calculatePentecost(int year)
{

	auto easterDate = calculateEaster(year);
	std::chrono::sys_days easterDay{easterDate};

	auto pentecostDay = easterDay + std::chrono::days{49};

	std::chrono::year_month_day pentecostDate{pentecostDay};

	return pentecostDate;

}

std::chrono::year_month_day calculateFirstAdventSunday(int year)
{

	std::chrono::year_month_day novDate{
		std::chrono::year{year},
			std::chrono::November,
			std::chrono::day{27}
	};

	std::chrono::sys_days calendarDay{novDate};

	std::chrono::weekday dayOfWeek{calendarDay};

	int daysUntilSunday = (7 - dayOfWeek.c_encoding()) % 7;

	std::chrono::sys_days adventDay =
		calendarDay + std::chrono::days{daysUntilSunday};

	std::chrono::year_month_day adventDate{adventDay};

	return adventDate;

}
