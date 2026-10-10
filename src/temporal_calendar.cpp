#include <chrono>
#include "temporal_calendar.h"
#include "easter.h"

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

