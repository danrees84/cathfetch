// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "scripture_summary.h"
#include "lectionary_loader.h"
#include "lectionary_resolver.h"
#include "calendar_selection.h"
#include "liturgical_resolver.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>

namespace
{
int checks = 0;
void require(bool condition, const char* message)
{
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
}
int main()
{
    try
    {
        using namespace std::chrono;
        const auto bible = loadBible();
        const auto lectionary = loadLectionary();
        const auto summaries = loadScriptureSummaries(lectionary, bible);
        require(summaries.entries.size() == lectionary.entries.size(), "Every stored reading set has a draft");
        std::ifstream input{"data/summaries/roman_summaries.json"};
        nlohmann::json document;
        input >> document;
        const auto rejectedData = [&](const nlohmann::json& bad) {
            bool failed = false;
            try { parseScriptureSummaries(bad, lectionary, bible); }
            catch (const std::exception&) { failed = true; }
            require(failed, "Malformed or conflicting summary data must fail");
        };
        auto bad = document;
        bad["entries"].push_back(bad["entries"][0]); rejectedData(bad);
        bad = document; bad["entries"][0]["reading_set_id"] = "unknown"; rejectedData(bad);
        bad = document; bad["entries"][0]["quotation_verification"] = "independently_verified"; rejectedData(bad);
        bad = document; bad["entries"][0]["editorial_status"] = "human_reviewed"; rejectedData(bad);
        bad = document; bad["entries"] = nlohmann::json::object(); rejectedData(bad);
        const auto calendar = loadSelectedCalendar("grc");
        const auto resolve = [&](year_month_day date, std::string_view name = "grc", std::string_view mass = "default") {
            return resolveMassReadings(resolveLiturgicalDay(date, calendar, name), lectionary, name, mass);
        };
        const auto advent = resolveScriptureSummary(resolve(2025y/November/30), summaries);
        require(advent.selected && advent.selected->reference == "Isaiah 2:5", "Advent A selects the curated Old Testament verse");
        require(advent.selected->quotation == "O house of Jacob, come ye, and let us walk in the light of the Lord.", "Pinned exact quotation");
        require(advent.selected->editorialStatus == "ai_draft", "No automatic human approval");
        const auto& original = *advent.selected;
        const auto daily = parseScriptureSummaries(document, lectionary, bible, original.readingSetId);
        require(daily.entries.size() == 1 && daily.entries.at(original.readingSetId).quotation == original.quotation,
                "Daily validation returns only the selected entry, with the same checked quotation");
        require(parseScriptureSummaries(document, lectionary, bible, "missing").entries.empty(),
                "A missing identity remains missing in selected-entry mode");
        const auto rejectDaily = [&](const nlohmann::json& changedDocument) {
            bool failed = false;
            try { parseScriptureSummaries(changedDocument, lectionary, bible, original.readingSetId); }
            catch (const std::exception&) { failed = true; }
            require(failed, "Selected-entry mode must reject invalid selected data and catalogue duplicates");
        };
        bad = document;
        bad["entries"].push_back(bad["entries"][0]);
        rejectDaily(bad);
        bad = document;
        for (auto& entry : bad["entries"])
            if (entry["reading_set_id"] == original.readingSetId) entry["quotation"] = "Invented quotation";
        rejectDaily(bad);
        // An unrelated entry is not used or validated on a daily invocation.
        // The complete parser, used by the test/export tools, still rejects it.
        bad = document;
        for (auto& entry : bad["entries"])
            if (entry["reading_set_id"] != original.readingSetId) { entry["quotation"] = "Invented quotation"; break; }
        require(parseScriptureSummaries(bad, lectionary, bible, original.readingSetId).entries.size() == 1,
                "Daily loading does not exhaustively validate unused commentary");
        rejectedData(bad);
        const auto reading = std::ranges::find(lectionary.entries, original.readingSetId, &ReadingSet::id);
        const auto rejected = [&](ScriptureSummary changed) {
            bool failed = false;
            try { validateScriptureSummary(changed, *reading, bible); }
            catch (const std::invalid_argument&) { failed = true; }
            require(failed, "Invalid summary must be rejected");
        };
        auto changed = original; changed.quotation += " invented"; rejected(changed);
        changed = original; changed.reference = "Isaiah 3:5"; rejected(changed);
        changed = original; changed.readingSetId = "nonexistent"; rejected(changed);
        changed = original; changed.category = "gospel"; rejected(changed);
        changed = original; changed.cycle = "C"; rejected(changed);
        changed = original; changed.mass = "vigil"; rejected(changed);
        changed = original; changed.testament = "new"; rejected(changed);
        changed = original; changed.referenceStatus = "current_edition"; rejected(changed);
        changed = original; changed.sourceXmlSha256.assign(64, '0'); rejected(changed);
        changed = original; changed.quotationSource = "https://example.com"; rejected(changed);
        changed = original; changed.editorialStatus = "human_reviewed"; rejected(changed);
        changed = original; changed.explanation = "Generic reassurance."; rejected(changed);
        changed = original; changed.rationale = "\033[31m"; rejected(changed);
        // Whole-verse display is more permissive than editorial membership.
        const auto partial = lookupScripture(bible, "John 1:1b-3a");
        require(!partial.verses[0].completeSourceVerse && partial.verses[1].completeSourceVerse &&
                !partial.verses[2].completeSourceVerse, "Partial boundaries cannot authorise a whole quotation");
        const auto merged = lookupScripture(bible, "Psalm 100:2-3", "hebrew");
        require(!merged.verses.front().completeSourceVerse, "Merged native verse is not automatically an exact source verse");
        const auto discontinuous = lookupScripture(bible, "John 1:1, 3-4");
        require(discontinuous.verses.size() == 3 && discontinuous.verses[1].verse == 3, "Omitted verses are not candidates");
        require(!resolveScriptureSummary(resolve(2026y/April/4), summaries).selected, "Holy Saturday has no daytime summary");
        require(resolveScriptureSummary(resolve(2026y/April/4, "grc", "vigil"), summaries).selected.has_value(), "Explicit Easter Vigil form");
        const auto transferred = resolveScriptureSummary(resolve(2024y/April/8), summaries);
        require(transferred.selected && transferred.selected->reference == "Luke 1:38", "Annunciation readings and summary follow the transfer");
        const auto corpus = resolveScriptureSummary(resolve(2038y/June/24), summaries);
        const auto john = resolveScriptureSummary(resolve(2038y/June/23), summaries);
        require(corpus.selected && corpus.selected->readingSetId == "corpus_christi:A:day", "Corpus Christi collision retains its assigned summary");
        require(john.selected && john.selected->readingSetId.starts_with("grc-06-24-"), "Existing provisional John transfer carries its summary");
        for (const auto date : {2026y/November/28, 2026y/November/29, 2024y/February/29, 2038y/June/24})
            require(resolveScriptureSummary(resolve(date), summaries).selected.has_value(), "Advent, leap day and coinciding solemnity boundary");
        const auto christmasNight = resolveScriptureSummary(resolve(2026y/December/25, "grc", "night"), summaries);
        const auto christmasDay = resolveScriptureSummary(resolve(2026y/December/25), summaries);
        require(christmasNight.selected && christmasDay.selected &&
                christmasNight.selected->readingSetId != christmasDay.selected->readingSetId, "Christmas forms select their own summary");
        for (const auto& assignment : lectionary.entries)
        {
            LectionaryDay selected;
            selected.status = ReadingStatus::Available;
            selected.selected = assignment;
            const auto summary = resolveScriptureSummary(selected, summaries);
            require(summary.selected && summary.selected->mass == assignment.mass &&
                    summary.selected->cycle == assignment.cycle, "Every stored cycle, alternative set and Mass form retains its own identity");
            if (!assignment.partial && std::ranges::find(assignment.calendars, "grc") != assignment.calendars.end())
            {
                const auto& chosen = *summary.selected;
                const auto slot = std::ranges::find(assignment.readings, chosen.category, &ReadingReference::role);
                for (const auto& alternative : slot->alternatives)
                {
                    const auto passage = lookupScripture(bible, alternative.reference, slot->psalmNumbering);
                    require(passage.bookId == chosen.bookId && std::ranges::any_of(passage.verses, [&](const auto& verse) {
                        return verse.completeSourceVerse && verse.chapter == chosen.chapter && verse.verse == chosen.verse &&
                               verse.text == chosen.quotation;
                    }), "The chosen verse remains appointed with each applicable alternative to its reading slot");
                }
            }
        }
        LectionaryDay missing;
        require(!resolveScriptureSummary(missing, summaries).selected, "Missing readings cannot acquire a summary");
        missing.status = ReadingStatus::Available;
        missing.selected = ReadingSet{};
        missing.selected->id = "no-such-summary";
        require(!resolveScriptureSummary(missing, summaries).selected, "Missing editorial data does not trigger generation");
        std::size_t dates = 0, available = 0, absent = 0, noMass = 0;
        for (const auto name : {"grc", "europe", "england", "wales"})
            for (int y = 2024; y <= 2030; ++y)
                for (sys_days date = year{y}/January/1; date < sys_days{year{y+1}/January/1}; date += days{1})
                {
                    const auto readings = resolve(year_month_day{date}, name);
                    const auto result = resolveScriptureSummary(readings, summaries);
                    ++dates;
                    if (readings.status == ReadingStatus::NoDaytimeMass) ++noMass;
                    else if (result.selected)
                    {
                        ++available;
                        require(result.selected->readingSetId == readings.selected->id, "Summary follows actual resolved reading identity");
                    }
                    else { ++absent; require(!result.diagnostic.empty(), "Editorial gaps are explicit"); }
                }
        require(absent == 0 && noMass == 28 && available == 10200, "Full v1 reference coverage, separate from verification");
        std::cout << "Scripture summaries: " << checks << " checks; " << summaries.entries.size() << " stored sets; "
                  << dates << " date/calendar pairs: " << available << " drafts, " << absent << " missing, " << noMass
                  << " without daytime Mass. Technical checks do not confer editorial approval.\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
