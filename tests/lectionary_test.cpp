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
        require(reference(resolve(2026y/November/30, "england"), "gospel") == "Matthew 4:18-22",
                "St Andrew uses the feast Gospel, not an Advent weekday");
        const auto adventMondayB = resolveMassReadings(resolveLiturgicalDay(2026y/November/30, {}), data);
        require(reference(adventMondayB, "first") == "Isaiah 2:1–5", "Advent Monday B without an observed feast");
        require(reference(resolve(2026y/April/27), "gospel") == "John 10:11–18", "Easter Monday Gospel conditional on Year A");
        require(reference(resolve(2025y/April/7), "gospel") == "John 8:12-20", "Lent Monday Gospel conditional on Year C");
        require(reference(resolve(2026y/April/6), "first") == "Acts 2:14, 22–33", "Easter Octave independently checked endpoint");
        require(reference(resolve(2026y/April/13), "first") == "Acts 4:23–31", "Missing source chapter independently restored");
        require(reference(resolve(2026y/March/19), "first") == "2 Sam 7:4-5a, 12-14a, 16" &&
                reference(resolve(2026y/March/19), "gospel") == "Matt 1:16, 18-21, 24a",
                "St Joseph retains independently collated subverse limits");
        require(reference(resolve(2026y/April/5), "first") == "Acts 10:34a, 37-43",
                "Easter daytime first reading has a partial introductory verse");
        require(reference(resolve(2026y/May/24), "second") == "1 Cor 12:3b-7, 12-13",
                "Pentecost starts with the selected part of verse three");
        require(reference(resolve(2026y/March/1), "first") == "Gen 12:1-4a" &&
                reference(resolve(2026y/March/1), "second") == "2 Tim 1:8b-10",
                "Lent A uses collated partial first and second readings");
        require(reference(resolve(2026y/February/20), "first") == "Isaiah 58:1-9a" &&
                reference(resolve(2026y/February/21), "first") == "Isaiah 58:9b-14",
                "Successive weekdays divide Isaiah verse nine rather than repeating its whole text");
        require(reference(resolve(2025y/November/8, "wales"), "gospel") == "Matt 5:1-12a",
                "Welsh All Saints default retains the general provision's Gospel endpoint");
        require(reference(resolve(2026y/April/21), "first") == "Acts 7:51–8:1a" &&
                reference(resolve(2026y/April/22), "first") == "Acts 8:1b-8",
                "Successive Easter weekdays retain the division within Acts eight verse one");
        require(reference(resolve(2026y/May/19), "gospel") == "John 17:1-11a" &&
                reference(resolve(2026y/May/20), "gospel") == "John 17:11b-19",
                "Successive Easter Gospels retain the division within John seventeen verse eleven");
        require(reference(resolve(2026y/July/11, "europe"), "gospel") == "Matthew 19:27–29",
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
        require(reference(resolve(2026y/January/26), "first") == "2 Timothy 1:1–8", "Timothy and Titus proper first reading");
        require(reference(resolve(2026y/January/26), "gospel") == "Mark 3:22–30", "Timothy and Titus retain weekday Gospel");
        require(reference(resolve(2026y/June/11), "first") == "Acts 11:21b–26; 13:1–3", "Barnabas proper first reading");
        require(reference(resolve(2024y/June/8), "gospel") == "Luke 2:41–51", "Immaculate Heart proper Gospel");
        require(resolve(2026y/June/13).key.starts_with("temporal:ordinary:"),
                "Immaculate Heart collision reduced to optional memorial does not impose proper readings");
        require(reference(resolve(2026y/August/29), "gospel") == "Mark 6:17–29", "John Baptist proper Gospel");
        require(reference(resolve(2026y/September/15), "gospel") == "John 19:25–27", "Sorrows proper Gospel");
        require(reference(resolve(2026y/October/2), "gospel") == "Matthew 18:1–5, 10", "Guardian Angels proper Gospel");
        require(resolve(2026y/August/22).key.starts_with("temporal:ordinary:"), "Queenship normally retains weekday readings");
        require(reference(resolve(2026y/April/23, "england"), "gospel") == "Luke 9:23–26", "George solemnity: correct saint identity");
        require(reference(resolve(2026y/September/24, "england"), "gospel") == "Luke 1:26–38", "Walsingham obligatory feast readings");
        require(reference(resolve(2026y/October/9, "wales"), "first") == "2 Timothy 4:1–5", "National Newman feast");
        require(resolve(2026y/October/9).key.starts_with("temporal:ordinary:"), "Roman optional Newman does not impose national feast readings");
        // Published diocesan ordos independently supply these literal weekday references.
        require(reference(resolve(2026y/August/25), "first") == "2 Thes 2:1-3a,14-17", "OT21 II first reading");
        require(reference(resolve(2026y/August/25), "gospel") == "Mt 23:23-26", "OT21 annual Gospel");
        require(reference(resolve(2025y/November/29), "first") == "Dn 7:15-27", "Final OT Saturday I");
        require(reference(resolve(2026y/November/28), "first") == "Rv 22:1-7", "Final OT Saturday II");
        require(reference(resolve(2026y/October/18), "second") == "1 Thess 1:1–5b",
                "OT29 A endpoint independently corroborated by Hallam and Universalis");
        for (int week = 21; week <= 34; ++week)
            for (int weekday = 1; weekday <= 6; ++weekday)
                for (std::string cycle : {"I", "II"})
                {
                    const auto key = "temporal:ordinary:" + std::to_string(week) + ":" + std::to_string(weekday);
                    const auto entry = std::ranges::find_if(data.entries, [&](const auto& set) {
                        return set.dayId == key && set.cycle == cycle;
                    });
                    require(entry != data.entries.end(), "All 168 late Ordinary Time weekday sets exist");
                    require(entry->verification == ReferenceVerification::PublishedTable,
                            "Foreign or older tables must not imply current England/Wales verification");
                }
        require(reference(resolve(2026y/March/2, "wales"), "first") == "Isaiah 61:1-3, 10-11", "Transferred David retains Welsh suggested first reading");
        require(reference(resolve(2026y/March/2, "wales"), "second") == "Philippians 3:8-14", "Welsh solemnity has three readings");
        require(resolve(2026y/March/2, "wales").selected->use == ReadingUse::Accommodated,
                "Suggested David set is an explicit Common choice");
        require(reference(resolve(2025y/March/1, "england"), "first") == "Philippians 3:8-14", "English David feast starts with Philippians");
        require(std::ranges::none_of(resolve(2025y/March/1, "england").selected->readings,
                    [](const auto& r) { return r.role == "second"; }), "English feast has two readings");
        require(reference(resolve(2025y/November/8, "wales"), "first") == "Rev 7:2-4,9-14", "All Saints of Wales uses directed All Saints selection");
        require(std::ranges::none_of(resolve(2025y/November/8, "wales").selected->readings,
                    [](const auto& r) { return r.role == "second"; }), "Welsh feast has two readings");
        require(resolve(2026y/November/8, "wales").key.starts_with("temporal:ordinary:"), "Welsh saints feast cannot displace Sunday");
        require(resolve(2025y/November/8, "england").key.starts_with("temporal:ordinary:"), "Welsh feast readings stay regional");
        require(resolve(2025y/October/25, "wales").status == ReadingStatus::MissingData, "Unverified Welsh martyrs proper remains explicit gap");
        require(reference(resolve(2026y/March/31), "gospel") == "John 13:21-33, 36-38", "Holy Tuesday discontinuity independently corrected");
        require(reference(resolve(2026y/June/23), "gospel") == "Matthew 7:6, 12-14", "OT12 Tuesday corrected Gospel");
        require(reference(resolve(2026y/August/19), "gospel") == "Matthew 20:1-16", "Whole appointed vineyard parable");
        require(reference(resolve(2026y/May/29), "gospel") == "Mark 11:11-25", "OT8 Friday corrected endpoint");
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
        minimal["entries"][0].erase("verification_sources");
        minimal["entries"][0].erase("verification_note");
        auto invalid = [&](auto mutate) { auto value = minimal; mutate(value); rejects([&] { parseLectionary(value); }); };
        invalid([](auto& j) { j["schema_version"] = 2; });
        invalid([](auto& j) { j["entries"][0]["cycle"] = "D"; });
        invalid([](auto& j) { j["entries"][0]["verification"] = "guessed"; });
        invalid([](auto& j) { j["entries"][0]["verification"] = "current_edition"; });
        auto evidenced = minimal;
        evidenced["entries"][0]["verification"] = "current_edition";
        evidenced["entries"][0]["verification_sources"] = {"https://example.org/current-edition"};
        evidenced["entries"][0]["verification_note"] = "Synthetic loader test evidence, not supplied data";
        require(parseLectionary(evidenced).entries[0].verification == ReferenceVerification::CurrentEdition,
                "Verification evidence round-trips");
        invalid([](auto& j) { j["entries"][0]["verification_sources"] = {"not-a-source"}; });
        invalid([](auto& j) { j["entries"][0]["readings"][0]["psalm_numbering"] = "guessed"; });
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
