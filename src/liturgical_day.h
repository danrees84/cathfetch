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
    std::optional<CelebrationRank> rank;
    PrecedenceLevel precedence;
    std::string observanceId;
    std::optional<LiturgicalColour> colour;
    std::optional<std::chrono::year_month_day> transferredFrom;
    // Optional memorials or restricted commemorations; the temporal day is default.
    std::vector<ObservanceCandidate> alternatives;
};

#endif
