#ifndef OBSERVANCE_CANDIDATE_H
#define OBSERVANCE_CANDIDATE_H

#include <optional>
#include <string>

#include "celebration_rank.h"
#include "liturgical_precedence.h"

struct ObservanceCandidate
{
    std::string id;
    std::string name;
    std::optional<CelebrationRank> rank;
    PrecedenceLevel precedence;
    bool optional = false;
};

#endif
