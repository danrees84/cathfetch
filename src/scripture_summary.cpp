#include "scripture_summary.h"
#include "data_path.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <fstream>
#include <sstream>
#include <set>
#include <stdexcept>
#include <string_view>

namespace
{
bool prose(const std::string& text)
{
    return text.find_first_not_of(" \t\r\n") != std::string::npos &&
        std::ranges::none_of(text, [](unsigned char c) { return c < 32 || c == 127; });
}

std::size_t words(const std::string& text)
{
    std::istringstream stream{text};
    std::size_t count = 0;
    for (std::string word; stream >> word;) ++count;
    return count;
}

nlohmann::json readSummaryDocument(const std::string& filename)
{
    const auto path = filename.empty() ? dataFile("summaries/roman_summaries.json") : std::filesystem::path{filename};
    std::ifstream file{path};
    if (!file) throw std::runtime_error("Could not open Scripture summaries: " + path.string());
    nlohmann::json document;
    file >> document;
    return document;
}
}

void validateScriptureSummary(const ScriptureSummary& summary, const ReadingSet& readings, const Bible& bible)
{
    const auto fail = [&](const std::string& reason) {
        throw std::invalid_argument("Invalid Scripture summary " + summary.readingSetId + ": " + reason);
    };
    if (summary.readingSetId != readings.id || summary.occasion != readings.dayId ||
        summary.cycle != readings.cycle || summary.mass != readings.mass)
        fail("reading-set identity or context differs");
    const auto reading = std::ranges::find(readings.readings, summary.category, &ReadingReference::role);
    if (reading == readings.readings.end() || reading->reference != summary.sourceReading ||
        summary.category == "acclamation" || summary.category == "procession_gospel")
        fail("source reading or category differs");
    const auto passage = lookupScripture(bible, reading->reference, reading->psalmNumbering);
    const bool included = passage.diagnostic.empty() && passage.bookId == summary.bookId &&
        std::ranges::any_of(passage.verses, [&](const auto& verse) {
            return verse.completeSourceVerse && verse.chapter == summary.chapter && verse.verse == summary.verse &&
                   verse.text == summary.quotation;
        });
    if (!included) fail("quotation is not a complete, checked verse within its appointed reading");
    const auto book = bible.books.find(summary.bookId);
    if (book == bible.books.end()) fail("unknown biblical book");
    const auto expectedReference = book->second.name + " " + std::to_string(summary.chapter) + ":" +
                                   std::to_string(summary.verse);
    if (summary.reference != expectedReference || summary.translation != bible.translation)
        fail("translation or native reference differs");
    // Stable biblical book IDs make this independent of English book names.
    // Deuterocanonical books belong to the Old Testament, not to this list.
    constexpr std::array<std::string_view, 27> newTestamentBooks{
        "MAT", "MRK", "LUK", "JHN", "ACT", "ROM", "1CO", "2CO", "GAL",
        "EPH", "PHP", "COL", "1TH", "2TH", "1TI", "2TI", "TIT", "PHM",
        "HEB", "JAS", "1PE", "2PE", "1JN", "2JN", "3JN", "JUD", "REV"
    };
    const bool isNewTestament = std::ranges::find(newTestamentBooks, summary.bookId) != newTestamentBooks.end();
    const auto testament = isNewTestament ? "new" : "old";
    if (summary.testament != testament) fail("testament classification differs");
    const auto expectedStatus = readings.verification == ReferenceVerification::CurrentEdition ?
        "current_edition" : "published_table";
    if (summary.referenceStatus != expectedStatus) fail("stale reference-verification status");
    if (!summary.quotationSource.starts_with("https://") || summary.sourceXmlSha256.size() != 64 ||
        summary.quotationSource != bible.source || summary.sourceXmlSha256 != bible.sourceXmlSha256)
        fail("quotation provenance is missing");
    for (const auto* field : {&summary.occasion, &summary.reference, &summary.quotation, &summary.translation,
                              &summary.explanation, &summary.liturgicalConnection, &summary.rationale})
        if (!prose(*field)) fail("blank prose or terminal control character");
    const auto wordCount = words(summary.explanation + " " + summary.liturgicalConnection);
    if (wordCount < 40 || wordCount > 70) fail("commentary must contain 40–70 words");
    if (summary.editorialStatus != "ai_draft" && summary.editorialStatus != "human_reviewed")
        fail("unknown editorial status");
    if (summary.editorialStatus == "human_reviewed" && (!prose(summary.reviewer) || !prose(summary.reviewDate)))
        fail("human review needs reviewer and date");
}

ScriptureSummaries loadScriptureSummaries(const Lectionary& lectionary, const Bible& bible, const std::string& filename)
{
    return parseScriptureSummaries(readSummaryDocument(filename), lectionary, bible);
}

ScriptureSummaries parseScriptureSummaries(const nlohmann::json& document, const Lectionary& lectionary,
                                         const Bible& bible, std::string_view readingSetId)
{
    if (document.at("schema_version") != 1) throw std::invalid_argument("Unsupported summary schema");
    if (document.at("policy") != "universal_v1") throw std::invalid_argument("Unsupported summary reading policy");
    if (!document.at("entries").is_array()) throw std::invalid_argument("Summary entries must be an array");
    ScriptureSummaries summaries;
    std::set<std::string> identities;
    for (const auto& item : document.at("entries"))
    {
        const std::string id = item.at("reading_set_id");
        if (!identities.insert(id).second) throw std::invalid_argument("Duplicate Scripture summary: " + id);
        if (!readingSetId.empty() && id != readingSetId) continue;
        ScriptureSummary summary;
        summary.readingSetId = item.at("reading_set_id");
        summary.occasion = item.at("occasion"); summary.cycle = item.at("cycle"); summary.mass = item.at("mass");
        summary.category = item.at("category"); summary.testament = item.at("testament");
        summary.sourceReading = item.at("source_reading"); summary.reference = item.at("reference");
        summary.bookId = item.at("book_id"); summary.chapter = item.at("chapter"); summary.verse = item.at("verse");
        summary.quotation = item.at("quotation"); summary.translation = item.at("translation");
        summary.quotationSource = item.at("quotation_source"); summary.sourceXmlSha256 = item.at("source_xml_sha256");
        summary.explanation = item.at("explanation"); summary.liturgicalConnection = item.at("liturgical_connection");
        summary.rationale = item.at("rationale"); summary.editorialStatus = item.at("editorial_status");
        if (item.contains("reviewer") && !item.at("reviewer").is_null()) summary.reviewer = item.at("reviewer");
        if (item.contains("review_date") && !item.at("review_date").is_null()) summary.reviewDate = item.at("review_date");
        summary.referenceStatus = item.at("reference_status");
        // Evidence is verified, never elevated merely by a JSON status string.
        if (item.at("quotation_verification") != "bundled_source_checked" ||
            item.at("technical_validation") != "checked")
            throw std::invalid_argument("Unknown summary validation claim: " + summary.readingSetId);
        const auto readings = std::ranges::find(lectionary.entries, summary.readingSetId, &ReadingSet::id);
        if (readings == lectionary.entries.end()) throw std::invalid_argument("Unknown summary reading set: " + summary.readingSetId);
        validateScriptureSummary(summary, *readings, bible);
        if (!summaries.entries.emplace(summary.readingSetId, std::move(summary)).second)
            throw std::invalid_argument("Duplicate Scripture summary");
    }
    return summaries;
}

ScriptureSummaryResult loadScriptureSummary(const LectionaryDay& day, const Lectionary& lectionary,
                                          const Bible& bible, const std::string& filename)
{
    if (day.status != ReadingStatus::Available || !day.selected)
        return resolveScriptureSummary(day, {});
    // Validate what will be displayed, including its exact quotation and source
    // reading. Exhaustive catalogue checks remain in tests and review tools.
    const auto summaries = parseScriptureSummaries(readSummaryDocument(filename), lectionary, bible, day.selected->id);
    return resolveScriptureSummary(day, summaries);
}

ScriptureSummaryResult resolveScriptureSummary(const LectionaryDay& day, const ScriptureSummaries& summaries)
{
    if (day.status == ReadingStatus::NoDaytimeMass)
        return {{}, "No daytime Mass; Easter Vigil available with --mass=vigil."};
    if (day.status != ReadingStatus::Available || !day.selected)
        return {{}, "Reading assignment unavailable for this day or Mass form."};
    const auto found = summaries.entries.find(day.selected->id);
    if (found == summaries.entries.end())
        return {{}, "No Scripture summary has been prepared for this reading set."};
    return {found->second, {}};
}
