// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#ifndef MOVABLE_OBSERVANCES_H
#define MOVABLE_OBSERVANCES_H

#include <chrono>
#include <vector>
#include <string_view>

#include "observance_candidate.h"

std::vector<ObservanceCandidate> findMovableObservances(
    std::chrono::year_month_day date,
    std::string_view calendarName = "grc");

#endif
