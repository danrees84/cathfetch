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

std::chrono::year_month_day calculateAscension(int year)
{
	auto easterDate = calculateEaster(year);

	std::chrono::sys_days easterDay{easterDate};

	std::chrono::sys_days ascensionDay =
		easterDay + std::chrono::days{39};

	return std::chrono::year_month_day{ascensionDay};
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

std::chrono::year_month_day calculateHolyFamily(int year)
{
	std::chrono::year_month_day christmasDate{
		std::chrono::year{year},
			std::chrono::December,
			std::chrono::day{25}
	};

	std::chrono::sys_days christmasDay{christmasDate};
	std::chrono::weekday christmasWeekday{christmasDay};

	if (christmasWeekday == std::chrono::Sunday)
	{
		return std::chrono::year{year} /
		std::chrono::December / 30;
	}

	int daysUntilSunday = 7 - christmasWeekday.c_encoding();

	return std::chrono::year_month_day{
		christmasDay + std::chrono::days{daysUntilSunday}
	};
}

std::chrono::year_month_day calculateBaptismOfLord(
		int year,
		std::string_view calendarName)
{
	auto epiphanyDate = calculateEpiphany(year, calendarName);

	std::chrono::sys_days epiphanyDay{epiphanyDate};
	std::chrono::weekday dayOfWeek{epiphanyDay};

	if (dayOfWeek == std::chrono::Sunday &&
			unsigned(epiphanyDate.day()) >= 7)
	{
		return std::chrono::year_month_day{
			epiphanyDay + std::chrono::days{1}
		};
	}

	int daysUntilSunday = 7 - dayOfWeek.c_encoding();

	return std::chrono::year_month_day{
		epiphanyDay + std::chrono::days{daysUntilSunday}
	};
}

std::chrono::year_month_day calculateEpiphany(
		int year,
		std::string_view calendarName)
{
	std::chrono::year_month_day januarySixth{
		std::chrono::year{year},
		std::chrono::January,
		std::chrono::day{6}
	};

	if (calendarName == "grc" || calendarName == "europe")
	{
		return januarySixth;
	}

	if (calendarName != "england" && calendarName != "wales")
	{
		throw std::invalid_argument("Unknown liturgical calendar");
	}

	std::chrono::sys_days calendarDay{januarySixth};
	std::chrono::weekday dayOfWeek{calendarDay};

	if (dayOfWeek == std::chrono::Saturday)
	{
		return std::chrono::year_month_day{
			calendarDay + std::chrono::days{1}
		};
	}

	if (dayOfWeek == std::chrono::Monday)
	{
		return std::chrono::year_month_day{
			calendarDay - std::chrono::days{1}
		};
	}

	return januarySixth;
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

bool isChristmas(
		std::chrono::year_month_day date,
		std::string_view calendarName)
{

	std::chrono::year_month_day christmasDay{
		date.year(),
		std::chrono::December,
		std::chrono::day{25}
	};

	auto baptismLordDate = calculateBaptismOfLord(
			int(date.year()), calendarName);

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

bool isOrdinaryTime(
		std::chrono::year_month_day date,
		std::string_view calendarName)
{
	return !isAdvent(date) &&
		!isChristmas(date, calendarName) &&
		!isLent(date) &&
		!isTriduum(date) &&
		!isEaster(date);
}

LiturgicalPeriod getLiturgicalPeriod(
		std::chrono::year_month_day date,
		std::string_view calendarName)
{
	if (isAdvent(date))
	{
		return LiturgicalPeriod::Advent;
	}

	if (isChristmas(date, calendarName))
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

int calculateOrdinaryTimeWeek(
		std::chrono::year_month_day date,
		std::string_view calendarName)
{

	if (!date.ok() || !isOrdinaryTime(date, calendarName))
	{
		throw std::invalid_argument("Date is not in Ordinary Time");
	}

	std::chrono::sys_days calendarDay{date};
	std::chrono::weekday dayOfWeek{calendarDay};
	std::chrono::sys_days weekSunday = calendarDay - std::chrono::days{int(dayOfWeek.c_encoding())};

	if (date < calculateAshWednesday(int(date.year())))
	{

		auto baptismOfLord = calculateBaptismOfLord(
				int(date.year()), calendarName);

		std::chrono::sys_days baptismDay{baptismOfLord};
		std::chrono::weekday baptismWeekday{baptismDay};

		std::chrono::sys_days baptismWeekSunday =
			baptismDay - std::chrono::days{
				int(baptismWeekday.c_encoding())
			};

		int weeksSinceBaptism =
			int((weekSunday - baptismWeekSunday).count() / 7);

		return weeksSinceBaptism + 1;
	}

		else {
			auto advent = calculateFirstAdventSunday(int(date.year()));
			std::chrono::sys_days adventDay{advent};
			int daysUntilAdvent = (adventDay - weekSunday).count();
			int weeksUntilAdvent = daysUntilAdvent / 7;
			return 35 - weeksUntilAdvent;
		}
	}

	int calculateAdventWeek(std::chrono::year_month_day date)
	{
		if (!date.ok() || !isAdvent(date))
		{
			throw std::invalid_argument("Date is not in Advent");
		}

		auto firstAdventSunday =
			calculateFirstAdventSunday(int(date.year()));

		std::chrono::sys_days firstSunday{firstAdventSunday};
		std::chrono::sys_days calendarDay{date};

		int daysSinceAdvent = (calendarDay - firstSunday).count();

		return (daysSinceAdvent / 7) + 1;
	}
