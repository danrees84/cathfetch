#ifndef SCRIPTURE_SUMMARY_H
#define SCRIPTURE_SUMMARY_H

#include "reading_set.h"
#include "scripture.h"
#include <map>
#include <optional>
#include <string>
#include <nlohmann/json_fwd.hpp>

// Prewritten editorial data. This module never generates commentary, selects
// Mass readings or calculates dates. The Lectionary resolver does that first.
struct ScriptureSummary
{
    std::string readingSetId;
    std::string occasion;
    std::string cycle;
    std::string mass;
    std::string category;
    std::string testament;
    std::string sourceReading;
    std::string reference; // Native Douay–Rheims address, shown with the quotation.
    std::string bookId;
    int chapter = 0;
    int verse = 0;
    std::string quotation;
    std::string translation;
    std::string quotationSource;
    std::string sourceXmlSha256;
    std::string explanation;
    std::string liturgicalConnection;
    std::string rationale;
    std::string editorialStatus; // ai_draft or human_reviewed, with evidence.
    std::string reviewer;
    std::string reviewDate;
    std::string referenceStatus;
};

struct ScriptureSummaries
{
    std::map<std::string, ScriptureSummary> entries;
};

struct ScriptureSummaryResult
{
    std::optional<ScriptureSummary> selected;
    std::string diagnostic;
};

ScriptureSummaries loadScriptureSummaries(const Lectionary& lectionary, const Bible& bible,
    const std::string& filename = "");
// Empty readingSetId validates the complete catalogue (tests and diagnostics).
// A specific ID validates only that entry, but still rejects duplicate identities.
ScriptureSummaries parseScriptureSummaries(const nlohmann::json& document, const Lectionary& lectionary,
    const Bible& bible, std::string_view readingSetId = {});
ScriptureSummaryResult loadScriptureSummary(const LectionaryDay& day, const Lectionary& lectionary,
    const Bible& bible, const std::string& filename = "");
void validateScriptureSummary(const ScriptureSummary& summary, const ReadingSet& readings, const Bible& bible);
ScriptureSummaryResult resolveScriptureSummary(const LectionaryDay& day, const ScriptureSummaries& summaries);

#endif
