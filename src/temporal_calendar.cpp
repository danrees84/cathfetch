#include <chrono>
#include "temporal_calendar.h"
#include "easter.h"
#include <iostream>
#include <stdexcept>

std::chrono::year_month_day calculateAshWednesday(int year)
{

	auto easterDate = calculateEaster(year);
	std::chrono::sys_days easterDay{easterDate};

	auto ashWednesdayDay = easterDay - std::chrono::days{46};

	std::chrono::year_month_day ashWednesdayDate{ashWednesdayDay};

	return ashWednesdayDate;

}

std::chrono::year_month_day calculateGoodFriday(int year)
{

	auto easterDate = calculateEaster(year);
	std::chrono::sys_days easterDay{easterDate};

	auto goodFridayDay = easterDay - std::chrono::days{2};

	std::chrono::year_month_day goodFridayDate{goodFridayDay};

	return goodFridayDate;

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

std::chrono::year_month_day calculateBaptismOfLord(int year)
{

	std::chrono::year_month_day janDate{
		std::chrono::year{year},
			std::chrono::January,
			std::chrono::day{7}
	};

	std::chrono::sys_days calendarDay{janDate};

	std::chrono::weekday dayOfWeek{calendarDay};

	int daysUntilSunday = (7 - dayOfWeek.c_encoding()) % 7;

	std::chrono::sys_days baptismDay =
		calendarDay + std::chrono::days{daysUntilSunday};

	std::chrono::year_month_day baptismDate{baptismDay};

	return baptismDate;

}

bool isAdvent(std::chrono::year_month_day date)
{

	auto firstAdventSunday = calculateFirstAdventSunday(int(date.year()));

	std::chrono::year_month_day christmasDay{
		date.year(),
			std::chrono::December,
			std::chrono::day{25}
	};

	return date >= firstAdventSunday && date < christmasDay;
}

bool isChristmas(std::chrono::year_month_day date)
{

	std::chrono::year_month_day christmasDay{
		date.year(),
			std::chrono::December,
			std::chrono::day{25}
	};


	auto baptismLordDate = calculateBaptismOfLord(int(date.year()));

	return date >= christmasDay || date <= baptismLordDate;

}

bool isLent(std::chrono::year_month_day date)
{

	std::chrono::year_month_day ashWednesday = calculateAshWednesday(int(date.year()));
	std::chrono::year_month_day goodFriday = calculateGoodFriday(int(date.year()));

	return date >= ashWednesday && date < goodFriday;
}


bool isTriduum(std::chrono::year_month_day date)
{

	std::chrono::year_month_day easter = calculateEaster(int(date.year()));
	std::chrono::year_month_day goodFriday = calculateGoodFriday(int(date.year()));

	return date >= goodFriday && date <= easter;

}

bool isEaster(std::chrono::year_month_day date)
{

	std::chrono::year_month_day easter = calculateEaster(int(date.year()));
	std::chrono::year_month_day pentecost = calculatePentecost(int(date.year()));

	return date >= easter && date <= pentecost;

}

bool isOrdinaryTime(std::chrono::year_month_day date)
{

	return !isAdvent(date) && !isChristmas(date) && !isLent(date) && !isTriduum(date) && !isEaster(date);

}


LiturgicalPeriod getLiturgicalPeriod(std::chrono::year_month_day date)
{
	if (isAdvent(date))
	{
		return LiturgicalPeriod::Advent;
	}

	if (isChristmas(date))
	{
		return LiturgicalPeriod::Christmas;
	}
	if (isLent(date))
	{
		return LiturgicalPeriod::Lent;
	}

	if (isEaster(date))
	{
		return LiturgicalPeriod::Easter;
	}
	if (isTriduum(date))
	{
		return LiturgicalPeriod::Triduum;
	}
	return	LiturgicalPeriod::OrdinaryTime;

}

int calculateOrdinaryTimeWeek(std::chrono::year_month_day date)
{

	if (!date.ok())
	{
		throw std::invalid_argument("Invalid calendar date");
	}

	if (!isOrdinaryTime(date))
	{
		throw std::invalid_argument("Date is not in Ordinary Time");
	}


	std::chrono::sys_days calendarDay{date};
	std::chrono::weekday dayOfWeek{calendarDay};
	std::chrono::sys_days weekSunday = calendarDay - std::chrono::days{int(dayOfWeek.c_encoding())};

	if (date < calculateAshWednesday(int(date.year())))
	{

		auto baptismOfLord = calculateBaptismOfLord(int(date.year()));

		std::chrono::sys_days baptismSunday{baptismOfLord};
		return 1 + (((weekSunday - baptismSunday).count())/7);
	}

	else {
		auto advent = calculateFirstAdventSunday(int(date.year()));
		std::chrono::sys_days adventDay{advent};
		int daysUntilAdvent = (adventDay - weekSunday).count();
		int weeksUntilAdvent = daysUntilAdvent / 7;
		return 35 - weeksUntilAdvent;
	}
}
