// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "calendar_lookup.h"

std::vector<FixedCelebration> findFixedCelebrations(
    const std::vector<FixedCelebration>& celebrations,
    std::chrono::month_day date)
{
    std::vector<FixedCelebration> matches;

    for (const auto& celebration : celebrations)
    {
        if (celebration.date == date)
        {
            matches.push_back(celebration);
        }
    }

    return matches;
}
