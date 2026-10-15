// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "celebration_rank.h"

#include <stdexcept>
#include <string>

CelebrationRank parseCelebrationRank(std::string_view value)
{
    if (value == "solemnity")
    {
        return CelebrationRank::Solemnity;
    }

    if (value == "feast")
    {
        return CelebrationRank::Feast;
    }

    if (value == "memorial")
    {
        return CelebrationRank::Memorial;
    }

    if (value == "optional_memorial")
    {
        return CelebrationRank::OptionalMemorial;
    }

    throw std::invalid_argument(
        "Unknown celebration rank: " + std::string{value});
}

std::string_view celebrationRankName(CelebrationRank rank)
{
    switch (rank)
    {
        case CelebrationRank::Solemnity:
            return "Solemnity";

        case CelebrationRank::Feast:
            return "Feast";

        case CelebrationRank::Memorial:
            return "Memorial";

        case CelebrationRank::OptionalMemorial:
            return "Optional Memorial";
    }

    throw std::invalid_argument("Unknown celebration rank");
}

std::string_view celebrationRankName(
    std::optional<CelebrationRank> rank)
{
    if (!rank.has_value())
    {
        return "No ordinary rank";
    }

    return celebrationRankName(rank.value());
}
