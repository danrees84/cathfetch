// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#ifndef LITURGICAL_RESOLVER_H
#define LITURGICAL_RESOLVER_H

#include <chrono>
#include <optional>
#include <vector>
#include <string_view>

#include "liturgical_day.h"
#include "fixed_celebration.h"

LiturgicalDay resolveLiturgicalDay(
    std::chrono::year_month_day date,
    const std::vector<FixedCelebration>& celebrations,
    std::string_view calendarName = "grc");


#endif
