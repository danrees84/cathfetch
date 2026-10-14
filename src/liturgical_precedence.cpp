#include "liturgical_precedence.h"
#include "temporal_calendar.h"
#include <stdexcept>
#include "easter.h"

bool outranks(PrecedenceLevel candidate, PrecedenceLevel current)
{
	return candidate < current;
}

PrecedenceLevel ordinaryTimeBaselinePrecedence(
    std::chrono::year_month_day date,
    std::string_view calendarName)
{


if (!date.ok() || !isOrdinaryTime(date, calendarName))
{
    throw std::invalid_argument(
        "Date is not a valid Ordinary Time date");
}

	std::chrono::sys_days calendarDay{date};
	std::chrono::weekday dayOfWeek{calendarDay};

	if (dayOfWeek == std::chrono::Sunday)
	{
		return PrecedenceLevel::SundayOfChristmasOrOrdinaryTime;
	}

	return PrecedenceLevel::OtherWeekday;
}

PrecedenceLevel temporalBaselinePrecedence(
    std::chrono::year_month_day date,
    std::string_view calendarName)
{
	if (!date.ok())
	{
		throw std::invalid_argument("Invalid calendar date");
	}

	if (date == calculateEpiphany(int(date.year()), calendarName))
    {
        return PrecedenceLevel::HighestDays;
    }

	if (isTriduum(date))
{
    return PrecedenceLevel::Triduum;
}


if (isOrdinaryTime(date, calendarName))
{
    return ordinaryTimeBaselinePrecedence(date, calendarName);
}

	if (isAdvent(date))
	{
		std::chrono::sys_days calendarDay{date};
		std::chrono::weekday dayOfWeek{calendarDay};

		if (dayOfWeek == std::chrono::Sunday)
		{
			return PrecedenceLevel::HighestDays;
		}

		if (date.month() == std::chrono::December &&
				unsigned(date.day()) >= 17)
		{
			return PrecedenceLevel::PrivilegedWeekday;
		}

		return PrecedenceLevel::OtherWeekday;
	}

	if (isLent(date))
{
    int year = int(date.year());

    if (date == calculateAshWednesday(year))
    {
        return PrecedenceLevel::HighestDays;
    }

    std::chrono::sys_days calendarDay{date};
    std::chrono::weekday dayOfWeek{calendarDay};

    if (dayOfWeek == std::chrono::Sunday)
    {
        return PrecedenceLevel::HighestDays;
    }

    std::chrono::sys_days palmSunday{
        calculatePalmSunday(year)
    };

    if (calendarDay > palmSunday)
    {
        return PrecedenceLevel::HighestDays;
    }

    return PrecedenceLevel::PrivilegedWeekday;
}

if (date == calculateAscension(int(date.year())))
{
    return PrecedenceLevel::HighestDays;
}

if (isEaster(date))
{
    std::chrono::sys_days calendarDay{date};
    std::chrono::weekday dayOfWeek{calendarDay};

    if (dayOfWeek == std::chrono::Sunday)
    {
        return PrecedenceLevel::HighestDays;
    }

    std::chrono::sys_days easterDay{
        calculateEaster(int(date.year()))
    };

    if (calendarDay <= easterDay + std::chrono::days{7})
    {
        return PrecedenceLevel::HighestDays;
    }

    return PrecedenceLevel::OtherWeekday;
}


if (isChristmas(date, calendarName))
{
    if (date.month() == std::chrono::December &&
        unsigned(date.day()) == 25)
    {
        return PrecedenceLevel::HighestDays;
    }

    std::chrono::sys_days calendarDay{date};
    std::chrono::weekday dayOfWeek{calendarDay};

    if (dayOfWeek == std::chrono::Sunday)
    {
        return PrecedenceLevel::SundayOfChristmasOrOrdinaryTime;
    }

    if ((date.month() == std::chrono::December &&
         unsigned(date.day()) >= 26) ||
        (date.month() == std::chrono::January &&
         unsigned(date.day()) == 1))
    {
        return PrecedenceLevel::PrivilegedWeekday;
    }

    return PrecedenceLevel::OtherWeekday;
}

	throw std::logic_error(
			"Temporal precedence not yet implemented for this season");
}

PrecedenceLevel fixedCelebrationPrecedence(
		const FixedCelebration& celebration)
{

	if (celebration.precedence.has_value())
{
    return *celebration.precedence;
}

	if (!celebration.rank.has_value())
	{
		throw std::invalid_argument(
				"Celebration has no ordinary rank: " + celebration.id);
	}

	if (celebration.rank.value() ==
			CelebrationRank::OptionalMemorial)
	{
		return PrecedenceLevel::OptionalMemorial;
	}

	if (celebration.rank == CelebrationRank::Memorial &&
			celebration.rankSourceCalendar == "general_roman")
	{
		return PrecedenceLevel::GeneralMemorial;
	}

	if (celebration.rank == CelebrationRank::Memorial &&
			celebration.rankSourceCalendar != "general_roman")
	{
		return PrecedenceLevel::ProperMemorial;
	}

	if (celebration.rank == CelebrationRank::Solemnity &&
			celebration.rankSourceCalendar != "general_roman")
	{
		return PrecedenceLevel::ProperSolemnity;
	}

	// TODO: Distinguish highest-ranking solemnities (level 2)
	// from ordinary General Roman Calendar solemnities (level 3).

	if (celebration.rank == CelebrationRank::Solemnity &&
			celebration.rankSourceCalendar == "general_roman")
	{
		return PrecedenceLevel::GeneralSolemnityOrAllSouls;
	}

	if (celebration.rank == CelebrationRank::Feast)
	{
		if (celebration.feastOfLord &&
				celebration.rankSourceCalendar == "general_roman")
		{
			return PrecedenceLevel::GeneralFeastOfLord;
		}

		if (celebration.rankSourceCalendar == "general_roman")
		{
			return PrecedenceLevel::GeneralFeast;
		}

        return PrecedenceLevel::ProperFeast;
	}

	throw std::logic_error(
			"Precedence not yet implemented for: " + celebration.id);
}
