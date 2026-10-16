// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "calendar_lookup.h"
#include "calendar_selection.h"
#include "liturgical_format.h"
#include "liturgical_resolver.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>

namespace
{
void require(bool condition, const std::string &message)
{
    if (!condition)
        throw std::runtime_error(message);
}
} // namespace

int main()
{
    try
    {
        using namespace std::chrono;
        using enum PrecedenceLevel;
        std::map<std::string_view, std::vector<FixedCelebration>> calendars;
        for (const auto name : {"grc", "england", "wales"})
            calendars.emplace(name, loadSelectedCalendar(name));
        const auto resolve = [&](year_month_day date, std::string_view calendar = "grc")
        { return resolveLiturgicalDay(date, calendars.at(calendar), calendar); };
        struct Case
        {
            year_month_day date;
            std::string_view calendar;
            std::string_view name;
            // Empty optionals mean this example does not check that field.
            std::optional<PrecedenceLevel> precedence;
            std::optional<CelebrationRank> rank;
            std::optional<LiturgicalPeriod> season;
            bool partialName = false;
        };
        const Case cases[]{
            {2025y / August / 10,
             "grc",
             "19th Sunday in Ordinary Time",
             SundayOfChristmasOrOrdinaryTime,
             {},
             {}},
            {2025y / November / 9, "grc", "The Dedication of the Lateran Basilica", {}, {}, {}},
            {2026y / October / 11, "grc", "28th Sunday in Ordinary Time", {}, {}, {}},
            {2025y / March / 1, "wales", "St David, Bishop, Patron of Wales", ProperSolemnity, {}, {}},
            {2025y / November / 10,
             "grc",
             "St Leo the Great, Pope and Doctor of the Church",
             GeneralMemorial,
             {},
             {}},
            {2026y / July / 23, "wales", "St Bridget, Religious", ProperFeast, {}, {}},
            {2026y / December / 8, "grc", "Immaculate Conception", GeneralSolemnityOrAllSouls, {}, {}, true},
            {2024y / December / 8, "grc", "Second Sunday of Advent", {}, {}, {}},
            {2024y / December / 9, "grc", "Immaculate Conception", GeneralSolemnityOrAllSouls, {}, {}, true},
            {2026y / December / 9, "grc", "Wednesday of the Second Week of Advent", {}, {}, {}},
            {2026y / December / 27,
             "grc",
             "The Holy Family of Jesus, Mary and Joseph",
             GeneralFeastOfLord,
             {},
             {}},
            {2025y / January / 6, "grc", "Epiphany", HighestDays, {}, {}, true},
            {2025y / January / 5, "england", "Epiphany", HighestDays, {}, {}, true},
            {2024y / January / 7, "grc", "The Baptism of the Lord", GeneralFeastOfLord, {}, {}},
            {2024y / January / 8, "england", "The Baptism of the Lord", GeneralFeastOfLord, {}, {}},
            {2026y / December / 25, "grc", "Nativity", HighestDays, {}, {}, true},
            {2026y / December / 30,
             "grc",
             "Sixth Day within the Octave of the Nativity of the Lord",
             PrivilegedWeekday,
             {},
             {}},
            {2026y / December / 26, "grc", "St Stephen, the First Martyr", GeneralFeast, {}, {}},
            {2027y / January / 1, "grc", "", GeneralSolemnityOrAllSouls, CelebrationRank::Solemnity, {}},
            {2027y / January / 3,
             "grc",
             "Second Sunday after Christmas",
             SundayOfChristmasOrOrdinaryTime,
             {},
             {}},
            {2025y / January / 6, "england", "Monday after Epiphany", {}, {}, {}},
            {2026y / May / 31,
             "grc",
             "The Most Holy Trinity",
             GeneralSolemnityOrAllSouls,
             CelebrationRank::Solemnity,
             {}},
            {2025y / June / 15,
             "grc",
             "The Most Holy Trinity",
             GeneralSolemnityOrAllSouls,
             CelebrationRank::Solemnity,
             {}},
            {2026y / June / 4,
             "grc",
             "The Most Holy Body and Blood of Christ",
             GeneralSolemnityOrAllSouls,
             CelebrationRank::Solemnity,
             {}},
            {2026y / June / 7,
             "england",
             "The Most Holy Body and Blood of Christ",
             GeneralSolemnityOrAllSouls,
             CelebrationRank::Solemnity,
             {}},
            {2026y / June / 12,
             "grc",
             "The Most Sacred Heart of Jesus",
             GeneralSolemnityOrAllSouls,
             CelebrationRank::Solemnity,
             {}},
            {2025y / June / 27,
             "grc",
             "The Most Sacred Heart of Jesus",
             GeneralSolemnityOrAllSouls,
             CelebrationRank::Solemnity,
             {}},
            {2026y / November / 22,
             "grc",
             "Our Lord Jesus Christ, King of the Universe",
             GeneralSolemnityOrAllSouls,
             CelebrationRank::Solemnity,
             {}},
            {2025y / November / 23,
             "grc",
             "Our Lord Jesus Christ, King of the Universe",
             GeneralSolemnityOrAllSouls,
             CelebrationRank::Solemnity,
             {}},
            {2025y / November / 2, "grc", "Faithful Departed", GeneralSolemnityOrAllSouls, {}, {}, true},
            {2026y / May / 24, "grc", "Pentecost Sunday", HighestDays, CelebrationRank::Solemnity, {}},
            {2026y / May / 14,
             "grc",
             "The Ascension of the Lord",
             HighestDays,
             CelebrationRank::Solemnity,
             {}},
            {2026y / April / 5,
             "grc",
             "Easter Sunday of the Resurrection of the Lord",
             Triduum,
             CelebrationRank::Solemnity,
             {}},
            {2026y / April / 6,
             "grc",
             "Monday within the Octave of Easter",
             HighestDays,
             {},
             LiturgicalPeriod::Easter},
            {2026y / April / 12,
             "grc",
             "Second Sunday of Easter (or of Divine Mercy)",
             HighestDays,
             {},
             LiturgicalPeriod::Easter},
            {2025y / May / 25, "grc", "Sixth Sunday of Easter", HighestDays, {}, LiturgicalPeriod::Easter},
            {2026y / April / 13,
             "grc",
             "Monday of the Second Week of Easter",
             OtherWeekday,
             {},
             LiturgicalPeriod::Easter},
            {2026y / May / 23,
             "grc",
             "Saturday of the Seventh Week of Easter",
             OtherWeekday,
             {},
             LiturgicalPeriod::Easter},
            {2026y / February / 18, "grc", "Ash Wednesday", HighestDays, {}, {}},
            {2026y / February / 19, "grc", "Thursday after Ash Wednesday", PrivilegedWeekday, {}, {}},
            {2026y / February / 22, "grc", "First Sunday of Lent", HighestDays, {}, {}},
            {2026y / February / 23, "grc", "Monday of the First Week of Lent", PrivilegedWeekday, {}, {}},
            {2026y / March / 29, "grc", "Palm Sunday of the Passion of the Lord", HighestDays, {}, {}},
            {2026y / March / 30, "grc", "Monday of Holy Week", HighestDays, {}, {}},
            {2026y / April / 2, "grc", "Holy Thursday", HighestDays, {}, {}},
            {2026y / April / 3, "grc", "Good Friday", Triduum, {}, {}},
            {2026y / April / 4, "grc", "Holy Saturday", Triduum, {}, {}},
        };
        for (const auto &test : cases)
        {
            const auto day = resolve(test.date, test.calendar);
            const auto context = formatCalendarDate(test.date) + " (" + std::string{test.calendar} + ")";
            if (!test.name.empty())
                require(test.partialName ? day.observanceName.find(test.name) != std::string::npos
                                         : day.observanceName == test.name,
                        context + ": unexpected name: " + day.observanceName);
            if (test.precedence)
                require(day.precedence == test.precedence, context + ": wrong precedence");
            if (test.rank)
                require(day.rank == test.rank, context + ": wrong rank");
            if (test.season)
                require(day.season == test.season, context + ": wrong season");
        }
        require(!resolve(2025y / November / 2).rank, "All Souls has no ordinary rank");
        require(resolve(2026y / June / 7).observanceName != "The Most Holy Body and Blood of Christ",
                "GRC Corpus Christi must not move to Sunday");
        require(resolve(2026y / June / 4, "england").observanceName !=
                    "The Most Holy Body and Blood of Christ",
                "England must not observe Corpus Christi on Thursday");

        auto reversed = calendars.at("wales");
        std::ranges::reverse(reversed);
        const auto forward = resolve(2026y / July / 23, "wales");
        const auto backward = resolveLiturgicalDay(2026y / July / 23, reversed, "wales");
        require(forward.observanceName == backward.observanceName &&
                    forward.precedence == backward.precedence,
                "Candidate order changed the Welsh observance");

        auto tied = findFixedCelebrations(calendars.at("grc"), November / 10);
        require(tied.size() == 1, "St Leo lookup");
        auto rival = tied.front();
        rival.id = "test-second-memorial";
        rival.name = "Test Second Memorial";
        tied.push_back(rival);
        bool rejected = false;
        try
        {
            resolveLiturgicalDay(2025y / November / 10, tied);
        }
        catch (const std::logic_error &)
        {
            rejected = true;
        }
        require(rejected, "Equal memorials need an explicit disposition");

        // A named temporal baseline must not hide a conflicting highest day.
        auto conflicting = calendars.at("grc");
        FixedCelebration intruder;
        intruder.id = "conflicting-highest-day";
        intruder.name = "Conflicting highest day";
        intruder.date = May / 14;
        intruder.rank = CelebrationRank::Solemnity;
        intruder.precedence = HighestDays;
        conflicting.push_back(intruder);
        const auto unresolved = resolveLiturgicalDay(2026y / May / 14, conflicting);
        require(!unresolved.unresolvedReason.empty() && unresolved.observanceId.empty() &&
                    unresolved.conflictingObservances.size() == 2,
                "Named baseline concealed a conflict");
        std::cout << "Observance resolver: " << std::size(cases) << " examples and conflict checks passed\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
