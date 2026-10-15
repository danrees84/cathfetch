// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#ifndef LITURGICAL_COLOUR_H
#define LITURGICAL_COLOUR_H
#include <optional>
#include "temporal_calendar.h"

enum class LiturgicalColour
{
    White,
    Red,
    Green,
    Violet,
    Rose,
    Black
};

std::optional<LiturgicalColour> seasonalColour(LiturgicalPeriod period);

#endif
