#include "liturgical_colour.h"

std::optional<LiturgicalColour> seasonalColour(LiturgicalPeriod period)
{
    switch (period)
    {
        case LiturgicalPeriod::Advent:
            return LiturgicalColour::Violet;

	case LiturgicalPeriod::Christmas:
	    return LiturgicalColour::White;

	case LiturgicalPeriod::Easter:
	    return LiturgicalColour::White;

	case LiturgicalPeriod::Lent:
	    return LiturgicalColour::Violet;

	case LiturgicalPeriod::OrdinaryTime:
	    return LiturgicalColour::Green;

	case LiturgicalPeriod::Triduum:
	    return std::nullopt;

    }

    return std::nullopt;
}
