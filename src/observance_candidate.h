#ifndef OBSERVANCE_CANDIDATE_H
#define OBSERVANCE_CANDIDATE_H

#include <chrono>
#include <optional>
#include <string>
#include "liturgical_colour.h"

#include "celebration_rank.h"
#include "fixed_celebration.h"
#include "liturgical_precedence.h"

// One possible observance, before the resolver compares it with the other
// candidates for the day. A candidate is not necessarily the final celebration.
struct ObservanceCandidate
{
    std::string id;
    std::string name;
    std::optional<CelebrationRank> rank = std::nullopt;
    PrecedenceLevel precedence;
    CelebrationSubject subject = CelebrationSubject::Other;
    bool principalPatron = false;
    std::optional<std::chrono::year_month_day> transferredTo = std::nullopt;
    bool optional = false;
    // Some celebrations name the seasonal day itself (for example, Easter),
    // rather than competing with a second, unnamed version of that same day.
    bool namesTemporalBaseline = false;
    std::optional<LiturgicalColour> colour = std::nullopt;
    std::optional<std::chrono::year_month_day> transferredFrom = std::nullopt;
    std::optional<std::chrono::year_month_day> scheduledFrom = std::nullopt;
    bool provisionalTransfer = false;
    std::string transferNote = {};
    std::string unresolvedReason = {};
    bool restrictedCommemoration = false;
    bool prevailsOnEqualPrecedence = false;
    bool reducesMemorialCollision = false;
};

#endif
