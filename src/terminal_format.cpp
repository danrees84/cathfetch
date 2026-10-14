#include "terminal_format.h"

std::string terminalColourCode(LiturgicalColour colour)
{
    switch (colour)
    {
        case LiturgicalColour::Green:
            return "\033[32m";

        case LiturgicalColour::Violet:
            return "\033[35m";

        case LiturgicalColour::Rose:
            return "\033[35m";

        case LiturgicalColour::Red:
            return "\033[31m";

        case LiturgicalColour::White:
            return "\033[37m";

        case LiturgicalColour::Black:
            return "\033[90m";
    }

    return "\033[0m";
}
