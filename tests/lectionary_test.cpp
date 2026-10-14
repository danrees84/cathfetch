#include "lectionary_cycle.h"
#include "lectionary_loader.h"
#include "lectionary_resolver.h"
#include "calendar_selection.h"
#include "liturgical_resolver.h"
#include <nlohmann/json.hpp>
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
template<class F> void rejects(F operation)
{
    bool rejected = false;
    try { operation(); } catch (const std::exception&) { rejected = true; }
    require(rejected, "Expected rejection");
}
std::string reference(const LectionaryDay& day, const std::string& role)
{
    require(day.status == ReadingStatus::Available && day.selected.has_value(), day.diagnostic);
    const auto reading = std::ranges::find(day.selected->readings, role, &ReadingReference::role);
    require(reading != day.selected->readings.end(), "Reading role absent: " + role);
    return reading->reference;
}
}

int main()
{
    try
    {
        // Independent published England/Wales years, including the civil-year overlap.
        for (auto [date, expected, weekday] : {
            std::tuple{2024y/December/1, SundayCycle::C, WeekdayCycle::I},
            std::tuple{2025y/November/30, SundayCycle::A, WeekdayCycle::II},
            std::tuple{2026y/November/28, SundayCycle::A, WeekdayCycle::II},
            std::tuple{2026y/November/29, SundayCycle::B, WeekdayCycle::I},
            std::tuple{2027y/November/28, SundayCycle::C, WeekdayCycle::II},
            std::tuple{2028y/December/3, SundayCycle::A, WeekdayCycle::I}})
        {
            const auto cycles = readingCycles(date);
            require(cycles.sunday == expected && cycles.weekday == weekday, "Published reading cycles");
        }
        require(readingCycles(2026y/November/29).liturgicalYear == 2027, "Ending-year convention");
        require(readingCycles(2028y/February/29).liturgicalYear == 2028, "Leap day");
        require(readingCycles(4099y/December/31).liturgicalYear == 4100, "Upper boundary Advent");
        rejects([] { readingCycles(2025y/February/29); });
        rejects([] { readingCycles(1582y/December/31); });
        rejects([] { readingCycles(4100y/January/1); });
        rejects([] { cycleName(static_cast<SundayCycle>(99)); });
        rejects([] { cycleName(static_cast<WeekdayCycle>(99)); });

        const auto data = loadLectionary();
        auto reversed = data;
        std::ranges::reverse(reversed.entries);
        const auto roman = loadSelectedCalendar("grc");
        const auto english = loadSelectedCalendar("england");
        const auto welsh = loadSelectedCalendar("wales");
        auto resolve = [&](year_month_day date, std::string_view calendar = "grc", std::string_view mass = "default") {
            return resolveMassReadings(resolveLiturgicalDay(date, calendar == "grc" ? roman :
                calendar == "england" ? english : welsh, calendar), data, calendar, mass);
        };
        // Source-table citations, not expected values generated from the resolver.
        require(reference(resolve(2025y/November/30), "first") == "Isa 2:1-5", "Advent A Isaiah");
        require(reference(resolve(2025y/November/30), "gospel") == "Matt 24:37-44", "Advent A Gospel");
        require(reference(resolve(2026y/November/29), "gospel") == "Mark 13:33-37", "Advent B Gospel");
        require(reference(resolve(2024y/December/1), "gospel") == "Luke 21:25-28, 34-36", "Advent C Gospel");
        require(reference(resolve(2025y/December/1), "first") == "Isaiah 4:2–6", "Advent Monday A conditional reading");
        require(resolve(2026y/November/30, "england").status == ReadingStatus::MissingData,
                "St Andrew is a missing proper, not a temporal substitution");
        const auto adventMondayB = resolveMassReadings(resolveLiturgicalDay(2026y/November/30, {}), data);
        require(reference(adventMondayB, "first") == "Isaiah 2:1–5", "Advent Monday B without an observed feast");
        require(reference(resolve(2026y/April/27), "gospel") == "John 10:11–18", "Easter Monday Gospel conditional on Year A");
        require(reference(resolve(2025y/April/7), "gospel") == "John 8:12-20", "Lent Monday Gospel conditional on Year C");
        require(reference(resolve(2026y/April/6), "first") == "Acts 2:14, 22–33", "Easter Octave independently checked endpoint");
        require(reference(resolve(2026y/April/13), "first") == "Acts 4:23–31", "Missing source chapter independently restored");
        require(resolve(2026y/July/11, "europe").status == ReadingStatus::MissingData,
                "European Benedict feast cannot inherit Roman memorial weekday policy");
        require(resolve(2026y/July/11).status == ReadingStatus::Available, "Roman Benedict memorial retains weekday readings");
        require(resolve(2026y/May/28, "wales").selected->dayId == "eternal_high_priest", "National proper feast");
        require(resolve(2025y/January/7).key == "temporal:after_epiphany:1", "Fixed Epiphany uses 7 January sequence");
        require(resolve(2025y/January/6, "england").key == "temporal:after_epiphany:1", "Sunday Epiphany uses weekday sequence");
        require(reference(resolve(2025y/January/13), "first") == "Hebrews 1:1–6", "Weekday I");
        require(reference(resolve(2026y/January/12), "first") == "1 Samuel 1:1–8", "Weekday II");
        require(reference(resolve(2025y/January/13), "gospel") == reference(resolve(2026y/January/12), "gospel"), "Common weekday Gospel");
        require(reference(resolve(2024y/December/9), "first") == "Gen 3:9-15, 20", "Transferred Immaculate Conception");
        require(reference(resolve(2026y/December/8), "gospel") == "Luke 1:26-38", "Solemnity overrides seasonal weekday");
        require(resolve(2038y/June/23).selected->dayId.find("06-24") != std::string::npos,
                "Provisional John transfer retains proper identity");
        const auto transferNote = resolveLiturgicalDay(2038y/June/23, roman).transferNote;
        const auto transferredReadings = resolve(2038y/June/23);
        require(!transferNote.empty() && std::ranges::find(transferredReadings.notes, transferNote) !=
                transferredReadings.notes.end(), "Transfer qualification retained");
        require(resolve(2038y/June/24).selected->dayId == "corpus_christi", "Lord solemnity proper");
        require(resolve(2026y/June/7, "england").selected->dayId == "corpus_christi", "National Sunday Corpus");
        require(resolve(2026y/June/4).selected->dayId == "corpus_christi", "General Thursday Corpus");
        require(resolve(2026y/January/2).key == "temporal:christmas:january:2", "Basil retains weekday readings");
        require(reference(resolve(2026y/July/29), "gospel") == "John 11:19–27", "Martha proper Gospel");
        require(resolve(2026y/July/29).selected->readings.size() >= 3, "Partial proper merged with weekday");
        require(resolve(2026y/January/26).status == ReadingStatus::MissingData, "Unknown proper provision is not assumed weekday");
        require(resolve(2026y/March/2, "wales").status == ReadingStatus::MissingData, "Unverified David solemnity is explicit gap");
        require(resolve(2026y/March/2, "england").key.starts_with("temporal:lent:"), "Omitted David has no festal readings");

        const auto christmas = resolve(2026y/December/25);
        require(christmas.selected->mass == "day" && christmas.alternatives.size() == 3, "Christmas default and forms");
        require(resolve(2026y/December/25, "grc", "night").selected->mass == "night", "Explicit Christmas Night");
        require(resolve(2026y/December/8, "grc", "night").status == ReadingStatus::MissingData, "Unavailable form does not fall back");
        require(resolve(2026y/April/4).status == ReadingStatus::NoDaytimeMass, "Holy Saturday daytime");
        const auto vigil = resolve(2026y/April/4, "grc", "vigil");
        require(reference(vigil, "gospel") == "Matthew 28: 1–10", "Easter Vigil A Gospel");
        require(vigil.selected->readings.size() >= 16, "All Easter Vigil readings and psalms retained");
        require(resolve(2026y/April/5).selected->mass == "day", "Easter Day default");
        require(resolve(2026y/April/3).selected->mass == "passion", "Good Friday liturgy, not invented Mass");
        require(resolve(2026y/April/2).selected->mass == "evening", "Holy Thursday evening qualification");
        require(resolve(2026y/May/24).selected->mass == "day" &&
                resolve(2026y/May/24, "grc", "vigil").selected->mass == "vigil", "Pentecost forms");
        require(resolve(2026y/May/24, "grc", "vigil-extended").selected->readings.size() == 10,
                "Extended Pentecost Vigil retains four reading/psalm pairs, epistle and Gospel");
        require(resolve(2026y/November/2).selected->use == ReadingUse::Accommodated,
                "All Souls suggested set is not presented as uniquely mandatory");
        const auto lent = resolve(2027y/February/28);
        require(std::ranges::any_of(lent.alternatives, [](const auto& set) { return set.alternativeOnly; }),
                "Year A Lent set preserved as an alternative in B");
        require(reference(resolve(2026y/February/2), "gospel") == "Lk 2:22-40", "Presentation weekday proper");
        require(resolve(2026y/February/2).selected->readings.size() == 4, "Feast weekday two readings plus psalm/acclamation");
        require(resolve(2025y/February/2).selected->readings.size() == 5, "Feast on Sunday three readings");

        auto unresolved = resolveLiturgicalDay(2026y/January/12, roman);
        unresolved.unresolvedReason = "Test conflict";
        require(resolveMassReadings(unresolved, data).status == ReadingStatus::UnresolvedDay, "Unresolved calendar protected");
        rejects([&] { resolve(2026y/January/12, "invalid"); });
        rejects([&] { resolve(2026y/January/12, "grc", "unknown"); });
        rejects([] { loadLectionary("/tmp/cathfetch-nonexistent-lectionary.json"); });

        std::ifstream file{"data/lectionary/roman_references.json"};
        nlohmann::json document; file >> document;
        auto minimal = document;
        minimal["entries"] = nlohmann::json::array({document["entries"][0]});
        minimal["memorials"] = nlohmann::json::array();
        auto invalid = [&](auto mutate) { auto value = minimal; mutate(value); rejects([&] { parseLectionary(value); }); };
        invalid([](auto& j) { j["schema_version"] = 2; });
        invalid([](auto& j) { j["entries"][0]["cycle"] = "D"; });
        invalid([](auto& j) { j["entries"][0]["verification"] = "guessed"; });
        invalid([](auto& j) { j["schema_version"] = 1.0; });
        invalid([](auto& j) { j["entries"][0]["source"] = ""; });
        invalid([](auto& j) { j["entries"][0]["readings"][0]["reference"] = "invented text"; });
        invalid([](auto& j) { j["entries"][0]["readings"][0]["role"] = "unknown"; });
        invalid([](auto& j) { j["entries"][0]["mass"] = "unknown"; });
        invalid([](auto& j) { j["entries"][0]["calendars"] = {"invalid"}; });
        invalid([](auto& j) { j["entries"][0]["lectionary_number"] = 1.5; });
        invalid([](auto& j) { j["entries"].push_back(j["entries"][0]); });
        invalid([](auto& j) { auto entry = j["entries"][0]; entry["id"] = "overlap"; entry["cycle"] = "A"; j["entries"].push_back(entry); });
        invalid([](auto& j) { j["entries"][0]["readings"].push_back(j["entries"][0]["readings"][0]); });

        std::map<ReadingStatus, int> totals;
        for (std::string_view calendar : {"grc", "europe", "england", "wales"})
        {
            const auto celebrations = loadSelectedCalendar(calendar);
            for (int year : {2008, 2022, 2024, 2025, 2026, 2027, 2028, 2029, 2030, 2038})
                for (sys_days date{std::chrono::year{year}/January/1};
                     date < sys_days{std::chrono::year{year+1}/January/1}; date += days{1})
                {
                    const auto day = resolveLiturgicalDay(year_month_day{date}, celebrations, calendar);
                    const auto readings = resolveMassReadings(day, data, calendar);
                    const auto otherOrder = resolveMassReadings(day, reversed, calendar);
                    require(readings.status == otherOrder.status && readings.key == otherOrder.key &&
                        (readings.selected ? readings.selected->id : "") == (otherOrder.selected ? otherOrder.selected->id : ""),
                        "Reading selection independent of data order");
                    require(readings.selected.has_value() == (readings.status == ReadingStatus::Available), "Status invariant");
                    require(readings.status != ReadingStatus::MissingData || !readings.diagnostic.empty(), "Gap has diagnostic");
                    ++totals[readings.status];
                }
        }
        std::cout << "Lectionary: " << checks << " checks; " << data.entries.size() << " sets; "
                  << totals[ReadingStatus::Available] << " available, " << totals[ReadingStatus::MissingData]
                  << " explicit gaps, " << totals[ReadingStatus::NoDaytimeMass] << " days without daytime Mass\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
