#include "liturgical_precedence.h"
#include "temporal_calendar.h"
#include <stdexcept>

bool outranks(PrecedenceLevel candidate, PrecedenceLevel current)
{
	return candidate < current;
}

PrecedenceLevel ordinaryTimeBaselinePrecedence(
		std::chrono::year_month_day date)
{
	if (!date.ok() || !isOrdinaryTime(date))
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

PrecedenceLevel fixedCelebrationPrecedence(
		const FixedCelebration& celebration)
{
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


// TODO (post-v1.0): Replace calendar-specific precedence
// assumptions with explicit classifications in JSON,
// allowing new jurisdictions without C++ changes.

		if (celebration.rankSourceCalendar == "europe")
		{
			return PrecedenceLevel::ProperFeast;
		}
	}

	throw std::logic_error(
			"Precedence not yet implemented for: " + celebration.id);
}

ObservanceStatus optionalMemorialStatusInOrdinaryTime(
		std::chrono::year_month_day date)
{
	auto baseline = ordinaryTimeBaselinePrecedence(date);

	if (outranks(baseline, PrecedenceLevel::OptionalMemorial))
	{
		return ObservanceStatus::Impeded;
	}

	return ObservanceStatus::Optional;
}

CelebrationAssessment assessOptionalMemorialInOrdinaryTime(
		const FixedCelebration& celebration,
		std::chrono::year_month_day date)
{
	if (!date.ok())
	{
		throw std::invalid_argument("Invalid calendar date");
	}

	if (celebration.date !=
			std::chrono::month_day{date.month(), date.day()})
	{
		throw std::invalid_argument(
				"Celebration does not match the given date");
	}

	if (celebration.rank != CelebrationRank::OptionalMemorial)
	{
		throw std::invalid_argument(
				"Celebration is not an optional memorial");
	}

	ObservanceStatus status =
		optionalMemorialStatusInOrdinaryTime(date);

	return CelebrationAssessment{celebration, status};
}
