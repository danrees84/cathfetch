// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#ifndef LECTIONARY_CYCLE_H
#define LECTIONARY_CYCLE_H

#include <chrono>
#include <string_view>

enum class SundayCycle { A, B, C };
enum class WeekdayCycle { I, II };

struct ReadingCycles
{
    int liturgicalYear; // Civil year in which this liturgical year ends.
    SundayCycle sunday;
    WeekdayCycle weekday;
};

ReadingCycles readingCycles(std::chrono::year_month_day date);
std::string_view cycleName(SundayCycle cycle);
std::string_view cycleName(WeekdayCycle cycle);

#endif
