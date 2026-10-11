#include "liturgical_format.h"

std::string liturgicalPeriodName(LiturgicalPeriod period)
{
    switch (period)
    {
        case LiturgicalPeriod::Advent:
            return "Advent";

case LiturgicalPeriod::Christmas:
            return "Christmas Time";
case LiturgicalPeriod::Easter:
            return "Easter Time";
case LiturgicalPeriod::Lent:
            return "Lent";
case LiturgicalPeriod::OrdinaryTime:
            return "Ordinary Time";
case LiturgicalPeriod::Triduum:
            return "Sacred Paschal Triduum";



    }

    return "Unknown";
}
