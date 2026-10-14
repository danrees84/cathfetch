#include "terminal_display.h"
#include "liturgical_format.h"
#include "terminal_format.h"

#include <iostream>
#include <stdexcept>

void printLiturgicalDay(const LiturgicalDay& day, bool showCross)
{
    if (!day.unresolvedReason.empty())
    {
        std::string diagnostic = formatCalendarDate(day.date) + ": " + day.unresolvedReason;
        for (const auto& conflict : day.conflictingObservances) diagnostic += "; " + conflict.name;
        throw std::logic_error(diagnostic);
    }
    if (showCross)
        std::cout << "          ███\n          ███          ";
    std::cout << formatCalendarDate(day.date) << '\n';
    if (showCross) std::cout << "      ███████████      ";
    if (day.colour)
        std::cout << terminalColourCode(*day.colour) << "●\033[0m ";
    std::cout << liturgicalPeriodName(day.season) << '\n';
    if (showCross) std::cout << "          ███          ";
    std::cout << day.observanceName;
    if (day.rank) std::cout << " · " << celebrationRankName(day.rank);
    if (day.provisionalTransfer) std::cout << " (provisional transfer)";
    std::cout << '\n';
    if (showCross)
        for (int i = 0; i < 4; ++i) std::cout << "          ███\n";
}
