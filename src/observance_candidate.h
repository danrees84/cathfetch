#ifndef OBSERVANCE_CANDIDATE_H
#define OBSERVANCE_CANDIDATE_H

#include <optional>
#include <string>
#include "liturgical_colour.h"

#include "celebration_rank.h"
#include "liturgical_precedence.h"

struct ObservanceCandidate
{
    std::string id;
    std::string name;
    std::optional<CelebrationRank> rank = std::nullopt;
    PrecedenceLevel precedence;
    bool optional = false;
    bool namesTemporalBaseline = false;
    std::optional<LiturgicalColour> colour = std::nullopt;
    std::optional<std::chrono::year_month_day> transferredFrom = std::nullopt;
    std::optional<std::chrono::year_month_day> scheduledFrom = std::nullopt;
    std::string unresolvedReason = {};
    bool restrictedCommemoration = false;
    bool prevailsOnEqualPrecedence = false;
    bool reducesMemorialCollision = false;
};

#endif
