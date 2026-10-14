#ifndef TERMINAL_DISPLAY_H
#define TERMINAL_DISPLAY_H

#include "liturgical_day.h"
#include <cstddef>

std::string renderLiturgicalDay(const LiturgicalDay& day, bool showCross, std::size_t columns);

void printLiturgicalDay(const LiturgicalDay& day, bool showCross);

#endif
