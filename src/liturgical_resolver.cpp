#include "liturgical_resolver.h"
#include "calendar_lookup.h"
#include "liturgical_precedence.h"
#include "liturgical_format.h"

#include <stdexcept>

LiturgicalDay resolveLiturgicalDay(
		std::chrono::year_month_day date,
		const std::vector<FixedCelebration>& celebrations,
		std::string_view calendarName)
{
	if (!date.ok())
	{
		throw std::invalid_argument("Invalid calendar date");
	}

	LiturgicalPeriod season = getLiturgicalPeriod(date, calendarName);

	if (season != LiturgicalPeriod::OrdinaryTime &&
			season != LiturgicalPeriod::Advent)
	{
		throw std::logic_error(
				"Resolution for this season not yet implemented");
	}

	PrecedenceLevel baseline =
		temporalBaselinePrecedence(date, calendarName);

	auto candidates = findFixedCelebrations(
			celebrations, date.month() / date.day());

	// Handle solemnities displaced by an Advent Sunday,
	// where the following Monday is available.

	std::chrono::sys_days calendarDay{date};
	std::chrono::weekday dayOfWeek{calendarDay};

	if (isAdvent(date) && dayOfWeek == std::chrono::Monday)
	{
		std::chrono::year_month_day previousDate{
			calendarDay - std::chrono::days{1}
		};

		auto previousCandidates = findFixedCelebrations(
				celebrations,
				previousDate.month() / previousDate.day());

		for (const auto& candidate : previousCandidates)
		{
			if (candidate.rank == CelebrationRank::Solemnity)
			{
				PrecedenceLevel sundayPrecedence =
					temporalBaselinePrecedence(previousDate);

				PrecedenceLevel solemnityPrecedence =
					fixedCelebrationPrecedence(candidate);

				if (outranks(sundayPrecedence, solemnityPrecedence))
				{
					candidates.push_back(candidate);
				}
			}
		}
	}

	std::optional<FixedCelebration> selectedCelebration;
	PrecedenceLevel bestPrecedence = baseline;
	bool tied = false;

	for (const auto& candidate : candidates)
	{
		if (candidate.rank == CelebrationRank::OptionalMemorial)
		{
			continue;
		}

		PrecedenceLevel candidatePrecedence =
			fixedCelebrationPrecedence(candidate);

		if (outranks(candidatePrecedence, bestPrecedence))
		{
			selectedCelebration = candidate;
			bestPrecedence = candidatePrecedence;
			tied = false;
		}
		else if (selectedCelebration.has_value() &&
				candidatePrecedence == bestPrecedence)
		{
			tied = true;
		}
	}

	if (tied)
	{
		throw std::logic_error(
				"Equal-precedence celebrations not yet resolved");
	}

	if (selectedCelebration.has_value())
	{
		return LiturgicalDay{
			date,
				season,
				selectedCelebration.value().name,
				selectedCelebration.value().rank,
				bestPrecedence
		};
	}

	std::string temporalName;

	if (season == LiturgicalPeriod::Advent)
	{
		temporalName = formatAdventDay(
				date, calculateAdventWeek(date));
	}
	else
	{
		temporalName = formatOrdinaryTimeDay(
				date, calculateOrdinaryTimeWeek(date, calendarName));
	}

	return LiturgicalDay{
		date,
			season,
			temporalName,
			std::nullopt,
			baseline
	};


}
