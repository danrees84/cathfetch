#include "liturgical_resolver.h"
#include "calendar_lookup.h"
#include "liturgical_precedence.h"
#include "liturgical_format.h"
#include "movable_observances.h"
#include "observance_candidate.h"
#include <string>
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
    season != LiturgicalPeriod::Advent &&
    season != LiturgicalPeriod::Christmas)
{
    throw std::logic_error(
        "Resolution for this season not yet implemented");
}

	bool isChristmasDay =
    season == LiturgicalPeriod::Christmas &&
    date.month() == std::chrono::December &&
    date.day() == std::chrono::day{25};

if (season != LiturgicalPeriod::OrdinaryTime &&
    season != LiturgicalPeriod::Advent &&
    season != LiturgicalPeriod::Christmas)
{
    throw std::logic_error(
        "Resolution for this season not yet implemented");
}


	PrecedenceLevel baseline =
		temporalBaselinePrecedence(date, calendarName);

	auto fixedCelebrations = findFixedCelebrations(
			celebrations, date.month() / date.day());

std::vector<ObservanceCandidate> candidates =
    findMovableObservances(date, calendarName);

for (const auto& celebration : celebrations)
{
    bool isEpiphany =
        celebration.id == "grc-01-06-the-epiphany-of-the-lord";

    std::chrono::month_day observedDate = celebration.date;

    if (isEpiphany)
    {
        auto epiphanyDate = calculateEpiphany(
            int(date.year()), calendarName);

        observedDate = epiphanyDate.month() / epiphanyDate.day();
    }

    if (observedDate != date.month() / date.day())
    {
        continue;
    }

    PrecedenceLevel precedence =
        fixedCelebrationPrecedence(celebration);

    if (isEpiphany)
    {
        precedence = PrecedenceLevel::HighestDays;
    }

    ObservanceCandidate candidate{
        celebration.id,
        celebration.name,
        celebration.rank,
        precedence,
        celebration.rank == CelebrationRank::OptionalMemorial
    };

    candidates.push_back(candidate);
}

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
					temporalBaselinePrecedence(previousDate, calendarName);

				PrecedenceLevel solemnityPrecedence =
					fixedCelebrationPrecedence(candidate);


				if (outranks(sundayPrecedence, solemnityPrecedence))
				{
					ObservanceCandidate transferredCandidate{
						candidate.id,
							candidate.name,
							candidate.rank,
							solemnityPrecedence,
							false
					};

					candidates.push_back(transferredCandidate);
				}


			}
		}
	}

std::optional<ObservanceCandidate> selectedCelebration;
PrecedenceLevel bestPrecedence = baseline;
bool tied = false;

if (isChristmasDay)
{
    for (const auto& candidate : candidates)
    {
        if (candidate.id ==
            "grc-12-25-the-nativity-of-the-lord-christmas")
        {
            selectedCelebration = candidate;
            selectedCelebration.value().precedence = baseline;
            break;
        }
    }

    if (!selectedCelebration.has_value())
    {
        throw std::logic_error(
            "Nativity of the Lord missing from calendar data");
    }
}



	for (const auto& candidate : candidates)
	{
		if (candidate.optional)
		{
			continue;
		}

		PrecedenceLevel candidatePrecedence = candidate.precedence;


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
else if (season == LiturgicalPeriod::Christmas)
{
    temporalName = formatChristmasDay(date, calendarName);
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
