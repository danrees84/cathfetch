// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#ifndef CALENDAR_SELECTION_H
#define CALENDAR_SELECTION_H

#include <string_view>
#include <vector>

#include "fixed_celebration.h"

std::vector<FixedCelebration> loadSelectedCalendar(
    std::string_view calendarName);

#endif
