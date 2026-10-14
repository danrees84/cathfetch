#ifndef LECTIONARY_RESOLVER_H
#define LECTIONARY_RESOLVER_H

#include "liturgical_day.h"
#include "reading_set.h"
#include <string_view>

// Supply a dataset returned by loadLectionary or checked by validateLectionary.
// Does not select celebrations or calculate their scheduled/transfer dates.
LectionaryDay resolveMassReadings(const LiturgicalDay& day, const Lectionary& lectionary,
                                 std::string_view calendarName = "grc",
                                 std::string_view mass = "default");
std::string temporalReadingKey(const LiturgicalDay& day, std::string_view calendarName);

#endif
