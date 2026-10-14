#ifndef LITURGICAL_DAY_H
#define LITURGICAL_DAY_H

#include <chrono>
#include <optional>
#include <string>

#include "temporal_calendar.h"
#include "celebration_rank.h"
#include "liturgical_precedence.h"
#include "observance_candidate.h"
#include <vector>

struct LiturgicalDay
{
    std::chrono::year_month_day date;
    LiturgicalPeriod season;
    std::string observanceName;
    std::optional<CelebrationRank> rank = std::nullopt;
    PrecedenceLevel precedence;
    std::string observanceId;
    std::optional<LiturgicalColour> colour = std::nullopt;
    std::optional<std::chrono::year_month_day> transferredFrom = std::nullopt;
    std::optional<std::chrono::year_month_day> scheduledFrom = std::nullopt;
    // An occurrence needing an authoritative disposition has no selected identity.
    std::string unresolvedReason;
    std::vector<ObservanceCandidate> conflictingObservances;
    // Optional memorials or restricted commemorations; the temporal day is default.
    std::vector<ObservanceCandidate> alternatives;
};

#endif
