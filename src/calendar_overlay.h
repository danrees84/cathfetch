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
