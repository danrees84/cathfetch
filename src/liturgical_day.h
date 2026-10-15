// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

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

// The result of resolving the calendar. The terminal displays this information;
// it does not decide which candidate wins or when a celebration transfers.
struct LiturgicalDay
{
    std::chrono::year_month_day date;
    LiturgicalPeriod season;
    std::string observanceName;
    // No value means no ordinary rank applies; it does not mean "unimportant".
    std::optional<CelebrationRank> rank = std::nullopt;
    PrecedenceLevel precedence;
    std::string observanceId;
    std::optional<LiturgicalColour> colour = std::nullopt;
    std::optional<std::chrono::year_month_day> transferredFrom = std::nullopt;
    std::optional<std::chrono::year_month_day> scheduledFrom = std::nullopt;
    bool provisionalTransfer = false;
    std::string transferNote = {};
    // An occurrence needing an authoritative disposition has no selected identity.
    std::string unresolvedReason;
    std::vector<ObservanceCandidate> conflictingObservances;
    std::vector<ObservanceCandidate> displacedObservances;
    // Optional memorials or restricted commemorations; the temporal day is default.
    std::vector<ObservanceCandidate> alternatives;
};

#endif
