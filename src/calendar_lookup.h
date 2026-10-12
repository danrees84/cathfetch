#ifndef CALENDAR_LOOKUP_H
#define CALENDAR_LOOKUP_H

#include <chrono>
#include <vector>
#include "fixed_celebration.h"

std::vector<FixedCelebration> findFixedCelebrations(
    const std::vector<FixedCelebration>& celebrations,
    std::chrono::month_day date);

#endif
