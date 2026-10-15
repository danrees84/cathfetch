// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#ifndef CALENDAR_OVERLAY_H
#define CALENDAR_OVERLAY_H

#include <string>
#include <vector>
#include "fixed_celebration.h"

void applyRankOverrides(
    std::vector<FixedCelebration>& celebrations,
    const std::string& filename);

void applyAdditions(
    std::vector<FixedCelebration>& celebrations,
    const std::string& filename);

void applyDateReassignments(
    std::vector<FixedCelebration>& celebrations,
    const std::string& filename);

void applyCalendarOverlay(
    std::vector<FixedCelebration>& celebrations,
    const std::string& filename);

#endif
