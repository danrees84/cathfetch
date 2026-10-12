#include "liturgical_resolver.h"
#include "calendar_lookup.h"
#include "liturgical_precedence.h"
#include "liturgical_format.h"

#include <stdexcept>
LiturgicalDay resolveLiturgicalDay(
    std::chrono::year_month_day date,
    const std::vector<FixedCelebration>& celebrations)
{
    if (!date.ok())
    {
        throw std::invalid_argument("Invalid calendar date");
    }

    if (!isOrdinaryTime(date))
    {
        throw std::logic_error(
            "Resolution outside Ordinary Time not yet implemented");
    }

    PrecedenceLevel baseline = ordinaryTimeBaselinePrecedence(date);

    auto candidates = findFixedCelebrations(
        celebrations, date.month() / date.day());

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
            LiturgicalPeriod::OrdinaryTime,
            selectedCelebration.value().name,
            selectedCelebration.value().rank,
            bestPrecedence
        };
    }

    return LiturgicalDay{
        date,
        LiturgicalPeriod::OrdinaryTime,
        formatOrdinaryTimeDay(date, calculateOrdinaryTimeWeek(date)),
        std::nullopt,
        baseline
    };
}
