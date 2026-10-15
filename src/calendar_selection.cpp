#include "calendar_selection.h"
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
        "data/calendars/general_roman_fixed.json");

    if (calendarName == "grc")
    {
        return celebrations;
    }

    applyCalendarOverlay(
        celebrations,
        "data/calendars/europe.json");

    if (calendarName == "europe")
    {
        return celebrations;
    }

    applyCalendarOverlay(
        celebrations,
        "data/calendars/england_wales.json");

    if (calendarName == "england")
    {
        applyCalendarOverlay(
            celebrations,
            "data/calendars/england.json");
    }
    else
    {
        applyCalendarOverlay(
            celebrations,
            "data/calendars/wales.json");
    }

    return celebrations;
}
