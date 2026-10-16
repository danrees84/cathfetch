// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

// Named examples check particular calendar rules. The full-year sweep below
// checks consistency and candidate-order independence; it is not an independent
// published calendar against which every date has been verified.
#include "calendar_selection.h"
#include "calendar_loader.h"
#include "calendar_overlay.h"
#include "liturgical_resolver.h"
#include "liturgical_format.h"
#include "easter.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>

namespace
{
using namespace std::chrono;
int checks = 0;
void require(bool condition, const std::string& message)
{
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
template<class F> void rejects(F operation, const std::string& message)
{
    bool rejected = false;
    try { operation(); } catch (const std::exception&) { rejected = true; }
    require(rejected, message);
}
}

int main()
{
    try
    {
        const auto roman = loadSelectedCalendar("grc");
        const auto europe = loadSelectedCalendar("europe");
        const auto england = loadSelectedCalendar("england");
        const auto wales = loadSelectedCalendar("wales");
        auto resolve = [&](year_month_day date, std::string_view calendar = "grc") {
            return resolveLiturgicalDay(date, calendar == "grc" ? roman :
                calendar == "europe" ? europe : calendar == "england" ? england : wales, calendar);
        };
        auto named = [&](year_month_day date, std::string_view name, std::string_view calendar = "grc") {
            const auto day = resolve(date, calendar);
            require(day.observanceName.find(name) != std::string::npos,
                    formatCalendarDate(date) + " (" + std::string(calendar) + "): expected " + std::string(name) + ", got " + day.observanceName);
            return day;
        };
        // Independent published dates, rather than expectations derived from computus.
        const year_month_day easters[]{2024y/March/31, 2025y/April/20, 2026y/April/5,
            2027y/March/28, 2028y/April/16, 2029y/April/1, 2030y/April/21,
            2008y/March/23, 1818y/March/22, 1943y/April/25, 2038y/April/25};
        for (auto date : easters)
        {
            require(calculateEaster(int(date.year())) == date, "Published Easter date");
            const auto day = named(date, "Easter Sunday of the Resurrection");
            require(day.precedence == PrecedenceLevel::Triduum && day.colour == LiturgicalColour::White,
                    "Easter precedence and colour");
        }
        // All thirteen table classes: real temporal days plus general/proper definitions.
        require(int(resolve(2026y/April/3).precedence) == 1, "Class 1");
        require(int(resolve(2026y/February/18).precedence) == 2, "Class 2");
        const CelebrationRank ranks[]{CelebrationRank::Solemnity, CelebrationRank::Solemnity,
            CelebrationRank::Feast, CelebrationRank::Feast, CelebrationRank::Feast,
            CelebrationRank::Memorial, CelebrationRank::Memorial, CelebrationRank::OptionalMemorial};
        const int levels[]{3,4,5,7,8,10,11,12};
        for (std::size_t i = 0; i < std::size(ranks); ++i)
        {
            FixedCelebration definition;
            definition.id = "test"; definition.name = "Test celebration";
            definition.date = October/6; definition.rank = ranks[i];
            if (levels[i] == 4 || levels[i] == 8 || levels[i] == 11) definition.rankSourceCalendar = "wales";
            definition.feastOfLord = levels[i] == 5;
            require(int(fixedCelebrationPrecedence(definition)) == levels[i], "Fixed class " + std::to_string(levels[i]));
            auto day = resolveLiturgicalDay(2026y/October/6, {definition}, "wales");
            require(int(day.precedence) == (levels[i] == 12 ? 13 : levels[i]), "Class selection");
            auto sunday = resolveLiturgicalDay(2026y/October/11, {definition}, "wales");
            require(int(sunday.precedence) == 6, "Unrelated weekday does not replace Sunday");
            definition.date = October/11;
            sunday = resolveLiturgicalDay(2026y/October/11, {definition}, "wales");
            require(int(sunday.precedence) == std::min(levels[i],6), "Sunday competition");
        }
        require(int(resolve(2026y/October/11).precedence) == 6, "Class 6");
        require(int(resolve(2026y/December/17).precedence) == 9, "Class 9");
        require(int(resolve(2026y/January/14).precedence) == 13, "Class 13");
        for (int i = 1; i <= 13; ++i)
            for (int j = 1; j <= 13; ++j)
                require(outranks(static_cast<PrecedenceLevel>(i), static_cast<PrecedenceLevel>(j)) == (i < j), "Strict precedence comparison");

        // Published ordo transfer reference cases: 2008, 2019, 2024–27.
        require(named(2024y/April/8, "Annunciation").transferredFrom == 2024y/March/25, "Annunciation transfer provenance");
        named(2008y/March/31, "Annunciation");
        named(2008y/March/15, "St Joseph");
        named(2016y/April/4, "Annunciation");
        named(2023y/March/20, "St Joseph");
        named(2027y/April/5, "Annunciation");
        named(2029y/April/9, "Annunciation");
        named(2024y/December/9, "Immaculate Conception");
        named(2024y/December/8, "Second Sunday of Advent");
        named(2025y/April/28, "St George", "england");
        named(2019y/April/30, "St George", "england");
        named(2030y/April/30, "St George", "england");
        named(2026y/March/2, "St David", "wales");
        require(resolve(2026y/March/2, "england").observanceId == "temporal", "English David feast is omitted, not transferred");
        named(2022y/June/23, "Nativity of St John");
        named(2022y/June/24, "Sacred Heart");
        const auto peter = named(2026y/June/28, "Peter and Paul", "england");
        require(peter.scheduledFrom == 2026y/June/29 && !peter.transferredFrom, "National scheduling is not impediment transfer");
        require(resolve(2026y/June/29, "england").observanceId == "temporal", "No duplicate Peter and Paul");
        named(2026y/June/29, "Peter and Paul", "grc");
        named(2026y/August/16, "Assumption", "wales");
        require(resolve(2026y/August/15, "wales").observanceId == "temporal", "No Assumption on national nominal Saturday");
        named(2027y/October/31, "All Saints", "england");
        named(2025y/November/2, "All Saints", "england");
        named(2025y/November/3, "Faithful Departed", "england");
        require(!named(2025y/November/2, "Faithful Departed").rank, "All Souls has no invented rank");
        named(2026y/May/25, "Mother of the Church");
        named(2028y/June/5, "Mother of the Church");
        named(2026y/May/28, "Eternal High Priest", "england");
        named(2021y/May/27, "Eternal High Priest", "england");
        require(resolve(2021y/May/25, "england").alternatives.empty(), "Obligatory Bede excludes other optional memorials");
        require(resolve(2026y/May/25).alternatives.empty(), "Mother of the Church excludes optional memorials");
        require(resolve(2026y/May/28).observanceId == "temporal", "High Priest is not universal");
        named(2026y/June/4, "Body and Blood");
        named(2026y/June/7, "Body and Blood", "wales");
        named(2024y/January/7, "Epiphany", "england");
        named(2024y/January/8, "Baptism", "england");
        named(2025y/January/5, "Epiphany", "wales");
        named(2026y/January/6, "Epiphany", "england");
        named(2022y/December/30, "Holy Family");
        require(resolve(2026y/January/20).alternatives.size() == 2, "Both optional memorials retained");
        require(resolve(2026y/June/13).alternatives.size() == 3, "Immaculate Heart collision and Saturday BVM");
        require(resolve(2025y/June/28).alternatives.size() == 3, "Immaculate Heart and Irenaeus collision");
        const auto lentMemorial = resolve(2026y/February/23);
        require(!lentMemorial.rank && lentMemorial.alternatives.size() == 1 &&
                lentMemorial.alternatives.front().restrictedCommemoration, "Lenten memorial reduced to commemoration");
        require(resolve(2026y/December/14).alternatives.empty(), "Obligatory memorial excludes optional choices");
        require(resolve(2026y/April/6).alternatives.empty(), "No optional choices in Easter octave");
        require(resolve(2026y/November/17, "england").alternatives.size() == 3, "England November 17 choice");
        require(resolve(2026y/November/17, "wales").rank == CelebrationRank::Memorial, "Wales retains Elizabeth memorial");
        require(resolve(2026y/May/5, "wales").alternatives.size() == 1, "Asaph is optional in current national calendar");
        require(resolve(2026y/August/26, "wales").alternatives.size() == 1, "David Lewis is optional");
        named(2026y/July/11, "St Benedict", "europe");
        require(resolve(2026y/July/11).rank == CelebrationRank::Memorial &&
                resolve(2026y/July/11, "europe").rank == CelebrationRank::Feast, "Same identity, different inherited rank");
        named(2026y/September/24, "Walsingham", "england");
        require(resolve(2026y/September/24, "wales").observanceId == "temporal", "Walsingham feast is England only");
        named(2026y/October/9, "Newman", "wales");
        require(resolve(2026y/October/10, "england").alternatives.size() == 4, "National reassignments retain all choices including Saturday BVM");
        require(named(2026y/April/3, "Good Friday").colour == LiturgicalColour::Red, "Good Friday red");
        require(named(2026y/May/24, "Pentecost").colour == LiturgicalColour::Red, "Pentecost red");
        require(named(2026y/August/10, "St Lawrence").colour == LiturgicalColour::Red, "Martyr feast red in Ordinary Time");
        require(resolve(2026y/November/18).alternatives.front().colour == LiturgicalColour::White, "Dedication of basilicas is white, not an apostle feast");
        require(named(2026y/December/27, "Holy Family").colour == LiturgicalColour::White, "Holy Family white");
        require(!resolve(2026y/April/4).colour && !resolve(2026y/April/2).colour, "No time-of-day colour invented");
        require(calculateOrdinaryTimeWeek(2026y/May/25) == 8, "Published post-Pentecost week");
        require(calculateOrdinaryTimeWeek(2026y/November/28) == 34, "Final Ordinary Time week");
        require(getLiturgicalPeriod(2026y/April/5) == LiturgicalPeriod::Easter && isTriduum(2026y/April/5), "Easter Sunday: Easter season and Triduum precedence");
        rejects([&]{resolveLiturgicalDay(2025y/February/29, roman);}, "Invalid non-leap date");
        rejects([&]{resolveLiturgicalDay(2026y/October/14, roman, "invalid");}, "Invalid calendar in resolver");
        rejects([&]{loadSelectedCalendar("invalid");}, "Invalid calendar in loader");
        rejects([&]{resolveLiturgicalDay(0y/January/1, roman);}, "Unsupported date range");
        auto duplicate = roman; duplicate.push_back(roman.front());
        rejects([&]{resolveLiturgicalDay(2026y/October/14, duplicate);}, "Duplicate identities rejected");
        auto contradictory = roman.front(); contradictory.name = "Contradiction"; duplicate.back() = contradictory;
        rejects([&]{resolveLiturgicalDay(2026y/October/14, duplicate);}, "Conflicting duplicate rejected");
        auto leo = *std::ranges::find_if(roman, [](const auto& c){return c.date == November/10;});
        auto rival = leo; rival.id = "rival";
        std::vector<FixedCelebration> tied{leo,rival};
        for (int i = 0; i < 2; ++i)
        {
            rejects([&]{resolveLiturgicalDay(2026y/November/10, tied);}, "Unspecified equal precedence rejected in either order");
            std::ranges::reverse(tied);
        }
        auto solemnity = roman.front();
        solemnity.date = October/6;
        auto secondSolemnity = solemnity; secondSolemnity.id = "second-solemnity";
        rejects([&]{resolveLiturgicalDay(2026y/October/6, {solemnity, secondSolemnity});}, "No arbitrary ordering between equal fixed solemnities");
        auto duplicateMovable = solemnity; duplicateMovable.id = "ascension"; duplicateMovable.date = May/14; duplicateMovable.rank = CelebrationRank::Feast;
        rejects([&]{resolveLiturgicalDay(2026y/May/14, {duplicateMovable});}, "Fixed and movable identities cannot duplicate");
        // Invalid JSON facts fail during loading; overlays apply transactionally.
        const std::string badFile = "/tmp/cathfetch-invalid-calendar-test.json";
        {std::ofstream file{badFile}; file << R"({"calendar":"general_roman","entries":[{"id":"x","name":"X","month":2,"day":30,"rank":"feast"}]})";}
        rejects([&]{loadFixedCelebrations(badFile);}, "Invalid fixed JSON date");
        {std::ofstream file{badFile}; file << R"({"calendar":"wales","additions":[],"overrides":[{"id":"missing","changes":{"rank":"feast"}}],"date_reassignments":[]})";}
        auto unchanged = roman;
        rejects([&]{applyCalendarOverlay(unchanged,badFile);}, "Unknown overlay target");
        require(unchanged.size() == roman.size() && unchanged.front().name == roman.front().name, "Failed overlay retains input");

        {std::ofstream file{badFile}; file << R"({"calendar":"wales","additions":[{"id":"temporary","name":"Temporary","month":1,"day":14,"rank":"feast"}],"overrides":[{"id":"missing","changes":{"rank":"feast"}}],"date_reassignments":[]})";}
        rejects([&]{applyCalendarOverlay(unchanged,badFile);}, "An invalid overlay must roll back an earlier addition");
        require(unchanged.size() == roman.size(), "Overlay addition was rolled back");
        {std::ofstream file{badFile}; file << R"({"calendar":"wales","additions":[],"overrides":[{"id":"grc-01-03-the-most-holy-name-of-jesus","changes":{"rank":"feast"}},{"id":"grc-01-03-the-most-holy-name-of-jesus","changes":{"rank":"memorial"}}],"date_reassignments":[]})";}
        rejects([&]{applyCalendarOverlay(unchanged,badFile);}, "Duplicate overrides cannot silently use their order");
        {std::ofstream file{badFile}; file << R"({"calendar":"wales","additions":[],"overrides":[],"date_reassignments":[{"id":"grc-01-03-the-most-holy-name-of-jesus","new_month":257,"new_day":3}]})";}
        rejects([&]{applyCalendarOverlay(unchanged,badFile);}, "Oversized month cannot wrap into a valid date");
        // Full-year resolution, boundary invariants, unique celebrations and order independence.
        int dates = 0;
        for (std::string_view calendar : {"grc", "europe", "england", "wales"})
        {
            const auto definitions = loadSelectedCalendar(calendar);
            auto reversed = definitions; std::ranges::reverse(reversed);
            std::vector<int> yearsToTest{1818, 1900, 1943};
            for (int year = 2000; year <= 2040; ++year) yearsToTest.push_back(year);
            for (int value : yearsToTest)
            {
                const year y{value};
                std::map<std::string,int> occurrences;
                for (sys_days date{y/January/1}; date < sys_days{(y+years{1})/January/1}; date += days{1})
                {
                    try
                    {
                        const auto day = resolveLiturgicalDay(year_month_day{date}, definitions, calendar);
                        const auto other = resolveLiturgicalDay(year_month_day{date}, reversed, calendar);
                        ++dates;
                        require(day.unresolvedReason.empty(), "Every tested calendar date resolves");
                        require(!day.observanceName.empty() && !day.observanceId.empty(), "Every date has identity");
                        require(day.observanceId == other.observanceId && day.precedence == other.precedence &&
                                day.rank == other.rank && day.colour == other.colour &&
                                day.transferredFrom == other.transferredFrom && day.scheduledFrom == other.scheduledFrom &&
                                day.alternatives.size() == other.alternatives.size() &&
                                day.provisionalTransfer == other.provisionalTransfer && day.transferNote == other.transferNote &&
                                day.displacedObservances.size() == other.displacedObservances.size(), "Candidate order independence");
                        for (std::size_t i = 0; i < day.alternatives.size(); ++i)
                            require(day.alternatives[i].id == other.alternatives[i].id, "Alternative order independence");
                        require(day.alternatives.empty() || day.precedence == PrecedenceLevel::PrivilegedWeekday ||
                                day.precedence == PrecedenceLevel::OtherWeekday, "Optional choices require an eligible temporal weekday");
                        if (day.observanceId != "temporal" && day.observanceId != "bvm_saturday")
                            require(++occurrences[day.observanceId] == 1, "Celebration repeated within year: " + day.observanceId);
                        if (day.observanceId == "temporal")
                            require((day.observanceName.find("Ordinary Time") != std::string::npos) ==
                                    (day.season == LiturgicalPeriod::OrdinaryTime), "No Ordinary Time fallback in another season");
                        if (isOrdinaryTime(year_month_day{date}, calendar))
                        {
                            const int week = calculateOrdinaryTimeWeek(year_month_day{date}, calendar);
                            require(week >= 1 && week <= 34, "Ordinary Time week range");
                        }
                        if (day.transferredFrom)
                            require(day.rank == CelebrationRank::Solemnity, "Only solemnities transfer by impediment");
                        require(day.colour.has_value() || date == sys_days{calculateEaster(value)}-days{1} ||
                                date == sys_days{calculateEaster(value)}-days{3}, "Resolved observance colour coverage");
                    }
                    catch (const std::exception& e)
                    {
                        throw std::runtime_error(std::string(calendar) + " " + formatCalendarDate(year_month_day{date}) + ": " + e.what());
                    }
                }
                for (const auto& definition : definitions)
                    if (definition.rank == CelebrationRank::Solemnity)
                    {
                        require(occurrences[definition.id] == 1, "Annual fixed solemnity coverage: " + definition.id);
                    }
                require(occurrences["easter_sunday"] == 1 && occurrences["pentecost"] == 1 &&
                        occurrences["ascension"] == 1, "Annual principal movable coverage");
            }
        }
        std::cout << "Calendar engine tests passed: " << checks << " checks; " << dates
                  << " dates across 44 full years and four calendars, each resolved in both candidate orders\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
