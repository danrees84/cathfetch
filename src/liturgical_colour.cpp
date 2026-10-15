// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "liturgical_colour.h"

std::optional<LiturgicalColour> seasonalColour(LiturgicalPeriod period)
{
    switch (period)
    {
        case LiturgicalPeriod::Advent:
        case LiturgicalPeriod::Lent:
            return LiturgicalColour::Violet;

        case LiturgicalPeriod::Christmas:
        case LiturgicalPeriod::Easter:
            return LiturgicalColour::White;

        case LiturgicalPeriod::OrdinaryTime:
            return LiturgicalColour::Green;

        case LiturgicalPeriod::Triduum:
            // Individual Triduum days have different colours; the season alone
            // cannot tell us which one to display.
            return std::nullopt;
    }
    return std::nullopt;
}
