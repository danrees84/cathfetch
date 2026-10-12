#ifndef TERMINAL_DISPLAY_H
#define TERMINAL_DISPLAY_H


#include <chrono>
#include <optional>
#include "fixed_celebration.h"

void printLiturgicalDay(
    std::chrono::year_month_day date,
    bool showCross,
    const std::optional<FixedCelebration>& observance = std::nullopt);

#endif
