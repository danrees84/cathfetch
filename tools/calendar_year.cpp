// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

// Diagnostic tool, separate from Cathfetch's terminal presentation. It writes
// tab-separated rows which can be saved for inspection, one row per civil date.
#include "calendar_selection.h"
#include "liturgical_resolver.h"
#include "liturgical_format.h"
#include <charconv>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
    try
    {
        if (argc < 2 || argc > 3) throw std::invalid_argument("Usage: calendar_year YEAR [grc|europe|england|wales]");
        const std::string_view input{argv[1]};
        int value = 0;
        const auto parsed = std::from_chars(input.data(), input.data() + input.size(), value);
        if (parsed.ec != std::errc{} || parsed.ptr != input.data() + input.size() || value < 1583 || value > 4099)
            throw std::invalid_argument("Year must be 1583–4099");
        const std::string_view calendar = argc == 3 ? argv[2] : "grc";
        const auto celebrations = loadSelectedCalendar(calendar);
        using namespace std::chrono;
        const year yearValue{value};
        std::cout << "date\tseason\tprecedence\trank\tcolour\tid\tobservance\ttransferred_from\tscheduled_from\talternatives\tdiagnostic\tprovisional_transfer\ttransfer_note\tdisplaced_observances\n";
        for (sys_days date{yearValue/January/1}; date < sys_days{(yearValue+years{1})/January/1}; date += days{1})
        {
            const auto day = resolveLiturgicalDay(year_month_day{date}, celebrations, calendar);
            std::cout << day.date << '\t' << liturgicalPeriodName(day.season) << '\t'
                      << int(day.precedence) << '\t';
            if (day.rank) std::cout << celebrationRankName(day.rank);
            std::cout << '\t';
            if (day.colour) std::cout << liturgicalColourName(*day.colour);
            std::cout << '\t' << day.observanceId << '\t' << day.observanceName << '\t';
            if (day.transferredFrom) std::cout << *day.transferredFrom;
            std::cout << '\t';
            if (day.scheduledFrom) std::cout << *day.scheduledFrom;
            std::cout << '\t';
            for (std::size_t i = 0; i < day.alternatives.size(); ++i)
            {
                if (i) std::cout << "; ";
                std::cout << day.alternatives[i].name;
                if (day.alternatives[i].restrictedCommemoration) std::cout << " (commemoration)";
            }
            std::cout << '\t' << day.unresolvedReason;
            for (const auto& conflict : day.conflictingObservances) std::cout << "; " << conflict.name;
            std::cout << '\t' << (day.provisionalTransfer ? "yes" : "no") << '\t' << day.transferNote << '\t';
            for (const auto& displaced : day.displacedObservances)
            {
                std::cout << displaced.name << " -> ";
                if (displaced.transferredTo) std::cout << *displaced.transferredTo;
                if (displaced.provisionalTransfer) std::cout << " (provisional)";
                std::cout << "; ";
            }
            std::cout << '\n';
        }
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
