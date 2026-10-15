#ifndef TERMINAL_DISPLAY_H
#define TERMINAL_DISPLAY_H

#include "liturgical_day.h"
#include "reading_set.h"
#include "scripture.h"
#include "scripture_summary.h"
#include <cstddef>

std::string renderLiturgicalDay(const LiturgicalDay& day, bool showCross, std::size_t columns,
                               const LectionaryDay* readings = nullptr, const Bible* bible = nullptr,
                               const ScriptureSummaryResult* summary = nullptr);

void printLiturgicalDay(const LiturgicalDay& day, bool showCross,
                        const LectionaryDay* readings = nullptr, const Bible* bible = nullptr,
                        const ScriptureSummaryResult* summary = nullptr);

#endif
