#ifndef LITURGICAL_DAY_H
#define LITURGICAL_DAY_H

#include <chrono>
#include <optional>
#include <string>

#include "temporal_calendar.h"
#include "celebration_rank.h"
#include "liturgical_precedence.h"

struct LiturgicalDay
{
    std::chrono::year_month_day date;
    LiturgicalPeriod season;
    std::string observanceName;
    std::optional<CelebrationRank> rank;
    PrecedenceLevel precedence;
};

#endif
