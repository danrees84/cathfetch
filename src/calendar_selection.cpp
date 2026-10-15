// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "calendar_selection.h"
#include "data_path.h"
#include "calendar_loader.h"
#include "calendar_overlay.h"

#include <stdexcept>
#include <string>

std::vector<FixedCelebration> loadSelectedCalendar(
    std::string_view calendarName)
{
    if (calendarName != "grc" &&
        calendarName != "europe" &&
        calendarName != "england" &&
        calendarName != "wales")
    {
        throw std::invalid_argument(
            "Unsupported calendar: " + std::string{calendarName});
    }

    // Inherit from broadest to most local: Roman, Europe, England & Wales,
    // then the selected country. Each overlay changes this same collection.
    auto celebrations = loadFixedCelebrations(
        dataFile("calendars/general_roman_fixed.json").string());

    if (calendarName == "grc")
    {
        return celebrations;
    }

    applyCalendarOverlay(
        celebrations,
        dataFile("calendars/europe.json").string());

    if (calendarName == "europe")
    {
        return celebrations;
    }

    applyCalendarOverlay(
        celebrations,
        dataFile("calendars/england_wales.json").string());

    if (calendarName == "england")
    {
        applyCalendarOverlay(
            celebrations,
            dataFile("calendars/england.json").string());
    }
    else
    {
        applyCalendarOverlay(
            celebrations,
            dataFile("calendars/wales.json").string());
    }

    return celebrations;
}
