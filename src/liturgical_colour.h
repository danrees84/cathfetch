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
