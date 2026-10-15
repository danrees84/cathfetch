// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#ifndef CALENDAR_LOOKUP_H
#define CALENDAR_LOOKUP_H

#include <chrono>
#include <vector>
#include "fixed_celebration.h"

std::vector<FixedCelebration> findFixedCelebrations(
    const std::vector<FixedCelebration>& celebrations,
    std::chrono::month_day date);

#endif
