#ifndef CALENDAR_SELECTION_H
#define CALENDAR_SELECTION_H

#include <string_view>
#include <vector>

#include "fixed_celebration.h"

std::vector<FixedCelebration> loadSelectedCalendar(
    std::string_view calendarName);

#endif
