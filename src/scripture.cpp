// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "scripture.h"
#include "data_path.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <utility>

namespace
{
std::string aliasKey(std::string_view value)
{
    std::string result;
    for (unsigned char c : value)
        if (std::isalnum(c)) result += char(std::tolower(c));
    return result;
}

void replaceAll(std::string& text, const std::string& from, const std::string& to)
{
    for (std::size_t at = 0; (at = text.find(from, at)) != std::string::npos; at += to.size())
        text.replace(at, from.size(), to);
}

bool psalmPair(const Bible& bible, int hebrew, int latin)
{
    if (hebrew < 1 || hebrew > 150 || latin < 1 || latin > 150) return false;
    const auto prefix = "hebrew:PSA:" + std::to_string(hebrew) + ":";
    auto mapping = bible.verseMappings.lower_bound(prefix);
    bool mapped = false;
    for (; mapping != bible.verseMappings.end() && mapping->first.starts_with(prefix); ++mapping)
    {
        mapped = true;
        for (const auto& target : mapping->second)
            if (target[0] == latin) return true;
    }
    return !mapped && hebrew == latin;
}

// An address may point to only part of a verse (for example, 3b). The bundled
// Bible contains whole verses, so partial selections need a display notice.
struct Address
{
    int chapter;
    int verse;
    bool partial;
};

Address endpoint(std::string& text, int chapter)
{
    static const std::regex pattern{R"(^\s*([0-9]+)(?:\s*:\s*([0-9]+))?([a-z]*))"};
    std::smatch match;
    if (!std::regex_search(text, match, pattern)) throw std::invalid_argument("Unsupported Scripture reference syntax");
    Address result{chapter, std::stoi(match[1]), !match[3].str().empty()};
    if (match[2].matched) result = {result.verse, std::stoi(match[2]), result.partial};
    if (result.chapter < 1 || result.verse < 1) throw std::invalid_argument("Invalid chapter or verse");
    text.erase(0, match.length());
    return result;
}

void trimLeft(std::string& text)
{
    text.erase(0, text.find_first_not_of(" \t\r\n"));
}

// Normalise typography and remove labels which are not part of the address.
// This only tidies syntax; it does not change chapter or verse numbering.
std::string normaliseReference(std::string_view reference)
{
    std::string text{reference};
    for (const auto& dash : {"–", "—", "−"}) replaceAll(text, dash, "-");
    replaceAll(text, " ", " ");
    replaceAll(text, " and ", ", ");
    static const std::regex volumeReference{R"(\s+\[[IVX]+:[0-9]+\]$)"};
    text = std::regex_replace(text, volumeReference, "");
    // The responsorial refrain is not another passage to look up.
    static const std::regex response{R"([rR]\.\s*|\s*℟)"};
    std::smatch match;
    if (std::regex_search(text, match, response)) text.erase(match.position());
    static const std::regex canticlePrefix{R"(^[Pp][Ss]\s+(?=(?:[1-3]\s*)?[A-Za-z]))"};
    text = std::regex_replace(text, canticlePrefix, "");
    while (!text.empty() && (std::isspace(static_cast<unsigned char>(text.back())) ||
                            text.back() == '.' || text.back() == ';')) text.pop_back();
    return text;
}


}

Bible loadBible(const std::string& filename)
{
    const auto path = filename.empty() ? dataFile("bibles/en_dra.json") : std::filesystem::path{filename};
    std::ifstream file{path};
    if (!file) throw std::runtime_error("Could not open offline Bible index: " + path.string() + "; run make all");
    nlohmann::json data;
    file >> data;
    if (data.at("schema_version") != 1) throw std::invalid_argument("Unsupported Bible index schema");
    Bible bible;
    bible.translation = data.at("translation");
    bible.source = data.at("source");
    bible.sourceXmlSha256 = data.at("xml_sha256");
    bible.omittedVerses = data.at("versification").at("omitted_verses").get<std::set<std::string>>();
    bible.passageMappings = data.value("passage_mappings", std::map<std::string, std::string>{});
    bible.passageDiagnostics = data.value("passage_diagnostics", std::map<std::string, std::string>{});
    for (const auto& [key, diagnostic] : bible.passageDiagnostics)
        if (diagnostic.empty() || bible.passageMappings.contains(key))
            throw std::invalid_argument("Conflicting or empty Bible passage diagnostic");
    for (const auto& [scheme, mappings] : data.at("versification").at("maps").items())
        for (const auto& [source, targets] : mappings.items())
        {
            bible.verseMappings.try_emplace(scheme + ":" + source);
            for (const auto& target : targets)
            {
                const auto book = source.substr(0, source.find(':'));
                if (target.at(0).get<std::string>() != book)
                    throw std::invalid_argument("Cross-book Bible mapping requires an explicit passage provision");
                bible.verseMappings[scheme + ":" + source].push_back({target.at(1).get<int>(), target.at(2).get<int>()});
            }
        }
    for (const auto& [source, targets] : data.at("verse_overrides").items())
    {
        auto& mapping = bible.verseMappings[source];
        mapping.clear();
        for (const auto& target : targets) mapping.push_back({target.at(1).get<int>(), target.at(2).get<int>()});
    }
    for (const auto& [source, targets] : bible.verseMappings)
    {
        const auto separator = source.find(':', source.find(':') + 1);
        const auto prefix = source.substr(0, separator + 1);
        for (const auto& target : targets)
        {
            const auto address = prefix + std::to_string(target[0]) + ":" + std::to_string(target[1]);
            if (address != source) bible.nonIdentityTargets.insert(address);
        }
    }
    for (const auto& [scheme, chapters] : data.at("versification").at("chapter_ends").items())
        for (const auto& [chapter, length] : chapters.items())
            bible.chapterEnds[scheme + ":" + chapter] = length.get<int>();
    for (const auto& [id, entry] : data.at("books").items())
    {
        BibleBook book;
        book.name = entry.at("name");
        book.numbering = entry.at("numbering");
        book.requiresExplicitMapping = entry.value("requires_explicit_mapping", false);
        book.nativeChapters = entry.value("native_chapters", std::set<int>{});
        for (const auto& [chapter, verses] : entry.at("chapters").items())
            for (const auto& [verse, text] : verses.items())
            {
                const int c = std::stoi(chapter), v = std::stoi(verse);
                if (c < 1 || v < 1 || !text.is_string() || text.get<std::string>().empty())
                    throw std::invalid_argument("Invalid Bible verse");
                book.chapters[c][v] = text.get<std::string>();
            }
        for (const auto& alias : entry.at("aliases"))
        {
            const auto key = aliasKey(alias.get<std::string>());
            const auto [found, inserted] = bible.aliases.emplace(key, id);
            if (key.empty() || (!inserted && found->second != id))
                throw std::invalid_argument("Conflicting Bible book alias");
        }
        if (book.chapters.empty()) throw std::invalid_argument("Empty Bible book");
        bible.books.emplace(id, std::move(book));
    }
    std::map<std::string, int> mappedEnds;
    for (const auto& [key, targets] : bible.verseMappings)
    {
        const auto separator = key.rfind(':');
        const auto chapterKey = key.substr(0, separator);
        auto& length = mappedEnds[chapterKey];
        length = std::max(length, std::stoi(key.substr(separator + 1)));
    }
    for (const auto& [chapterKey, mappedEnd] : mappedEnds)
    {
        const auto first = chapterKey.find(':');
        const auto last = chapterKey.rfind(':');
        const auto& book = bible.books.at(chapterKey.substr(first + 1, last - first - 1));
        const auto chapter = book.chapters.find(std::stoi(chapterKey.substr(last + 1)));
        const int nativeEnd = chapter == book.chapters.end() ? 0 : chapter->second.rbegin()->first;
        bible.chapterEnds.try_emplace(chapterKey, std::max(mappedEnd, nativeEnd));
    }
    if (bible.books.size() != 73) throw std::invalid_argument("Offline Bible must contain all 73 books");
    return bible;
}

namespace
{
ScripturePassage lookupPassage(const Bible& bible, std::string_view reference,
                              std::string_view psalmNumbering, bool nativeMapping)
{
    ScripturePassage result;
    try
    {
        std::string text = normaliseReference(reference);
        std::smatch match;
        // Traditional tables sometimes cite a complete psalm, or omit the
        // chapter colon. Single-chapter letters also omit their chapter.
        static const std::regex wholePsalm{R"(^\s*(Ps|Psalm)\s*([0-9]+)$)", std::regex::icase};
        if (std::regex_match(text, match, wholePsalm))
        {
            const auto c = std::stoi(match[2]);
            const auto scheme = psalmNumbering == "hebrew" ? "hebrew" : "greek";
            const auto end = bible.chapterEnds.find(std::string{scheme} + ":PSA:" + std::to_string(c));
            const auto& contents = bible.books.at("PSA").chapters;
            const int length = end != bible.chapterEnds.end() ? end->second : contents.at(c).rbegin()->first;
            text = "Ps " + std::to_string(c) + ":1-" + std::to_string(length);
        }
        static const std::regex missingPsalmColon{R"(^((?:Ps|Psalm)\s*[0-9]+)\s+([0-9]))", std::regex::icase};
        text = std::regex_replace(text, missingPsalmColon, "$1:$2");
        static const std::regex singleChapter{R"(^\s*((?:[1-3]\s*)?[A-Za-z]+(?:\s+[A-Za-z]+)*)\s+([0-9]+.*)$)"};
        if (text.find(':') == std::string::npos && std::regex_match(text, match, singleChapter))
        {
            const auto alias = bible.aliases.find(aliasKey(match[1].str()));
            if (alias != bible.aliases.end() && bible.books.at(alias->second).chapters.size() == 1)
                text = match[1].str() + " 1:" + match[2].str();
        }
        static const std::regex wholeChapter{R"(^\s*((?:[1-3]\s*)?[A-Za-z]+(?:\s+[A-Za-z]+)*)\s+([0-9]+)$)"};
        if (std::regex_match(text, match, wholeChapter))
        {
            const auto alias = bible.aliases.find(aliasKey(match[1].str()));
            if (alias != bible.aliases.end())
            {
                const auto& book = bible.books.at(alias->second);
                const int c = std::stoi(match[2]);
                const auto end = bible.chapterEnds.find(book.numbering + ":" + alias->second + ":" + std::to_string(c));
                const int length = end != bible.chapterEnds.end() ? end->second : book.chapters.at(c).rbegin()->first;
                text += ":1-" + std::to_string(length);
            }
        }
        // Older Catholic tables split the combined LXX psalms into A/B.
        static const std::regex splitPsalm{R"(^\s*(?:Ps|Psalm)\s*(113[AaBb]|9[Bb])\s*:(.*)$)", std::regex::icase};
        if (std::regex_match(text, match, splitPsalm))
        {
            const auto label = match[1].str();
            const int hebrew = label == "113A" || label == "113a" ? 114 :
                               label == "113B" || label == "113b" ? 115 : 10;
            text = "Ps " + std::to_string(hebrew) + ":" + match[2].str();
            psalmNumbering = "hebrew";
        }
        static const std::regex heading{R"(^\s*((?:[1-3]\s*)?[A-Za-z]+(?:\s+[A-Za-z]+)*)\s*:?\s*([0-9]+)(?:\s*\(([0-9]+)\))?\s*:\s*(.*)$)"};
        if (!std::regex_match(text, match, heading)) throw std::invalid_argument("Unsupported Scripture reference syntax");
        const auto alias = bible.aliases.find(aliasKey(match[1].str()));
        if (alias == bible.aliases.end()) throw std::invalid_argument("Book name is not recognised");
        const auto& book = bible.books.at(alias->second);
        const bool psalm = alias->second == "PSA";
        int chapter = std::stoi(match[2]);
        std::string key = alias->second + std::to_string(chapter) + ":" + match[4].str();
        key.erase(std::remove_if(key.begin(), key.end(), [](unsigned char c) { return std::isspace(c); }), key.end());
        replaceAll(key, ".", ",");
        if (psalmNumbering != "unspecified" && psalmNumbering != "lxx" && psalmNumbering != "hebrew")
            throw std::invalid_argument("Unknown psalm numbering convention");
        if (match[3].matched)
        {
            if (!psalm) throw std::invalid_argument("Dual numbering is supported only for psalms");
            const int other = std::stoi(match[3]);
            // Common H(LXX) and LXX(H) notations are both accepted, only when
            // the pair can be verified. Verse alignment still needs a policy.
            if (psalmPair(bible, chapter, other)) psalmNumbering = "hebrew";
            else if (psalmPair(bible, other, chapter)) psalmNumbering = "lxx";
            else throw std::invalid_argument("Unrecognised dual psalm numbering");
        }
        // Prefer a checked whole-passage correspondence to individual verse
        // mappings. The recursive call uses native numbers, so it cannot apply
        // this source-to-native conversion for a second time.
        if (!nativeMapping)
        {
            const auto mappingKey = psalm ? std::string{psalmNumbering} + ":" + key : key;
            const auto mapped = bible.passageMappings.find(mappingKey);
            if (mapped != bible.passageMappings.end())
            {
                auto passage = lookupPassage(bible, mapped->second, "lxx", true);
                // A whole-passage correspondence does not establish that an
                // individual native verse fits an individual source verse.
                for (auto& verse : passage.verses) verse.completeSourceVerse = false;
                if (passage.diagnostic.empty()) passage.numberingNote = "Douay–Rheims numbering is used below.";
                return passage;
            }
            const auto diagnostic = bible.passageDiagnostics.find(key);
            if (diagnostic != bible.passageDiagnostics.end())
            {
                throw std::invalid_argument(diagnostic->second);
            }
        }
        text = match[4];
        // A merged verse or overlapping subverse may be selected twice; print it
        // once, in the order in which the reference first requests it.
        std::set<std::pair<int, int>> seen;
        while (!text.empty())
        {
            auto first = endpoint(text, chapter);
            chapter = first.chapter;
            trimLeft(text);
            auto last = first;
            if (!text.empty() && text.front() == '-')
            {
                text.erase(0, 1);
                trimLeft(text);
                if (!text.empty() && text.front() >= 'a' && text.front() <= 'z')
                {
                    // 1b–e means parts of verse 1, rather than a new verse.
                    last.partial = true;
                    text.erase(0, text.find_first_not_of("abcdefghijklmnopqrstuvwxyz"));
                }
                else last = endpoint(text, chapter);
                chapter = last.chapter;
            }
            if (last.chapter < first.chapter ||
                (last.chapter == first.chapter && last.verse < first.verse) || last.chapter - first.chapter > 150)
                throw std::invalid_argument("Reversed or excessive Scripture range");
            result.includesWholeVerses |= first.partial || last.partial;
            for (int c = first.chapter; c <= last.chapter; ++c)
            {
                std::string scheme = nativeMapping || book.nativeChapters.contains(c) ? "latin" : book.numbering;
                if (psalm && !nativeMapping)
                {
                    if (psalmNumbering == "unspecified")
                        throw std::invalid_argument("Psalm numbering has not been identified for this reference");
                    scheme = psalmNumbering == "hebrew" ? "hebrew" : "greek";
                }
                const auto chapterKey = scheme + ":" + alias->second + ":" + std::to_string(c);
                const auto sourceEnd = bible.chapterEnds.find(chapterKey);
                const auto sameChapter = book.chapters.find(c);
                const int length = sourceEnd != bible.chapterEnds.end() ? sourceEnd->second :
                    sameChapter != book.chapters.end() ? sameChapter->second.rbegin()->first : 0;
                const int start = c == first.chapter ? first.verse : 1;
                const int end = c == last.chapter ? last.verse : length;
                if (length == 0 || start > length || end > length)
                    throw std::invalid_argument("Verse is absent from the source numbering scheme");
                if (end < start || end - start > 2000) throw std::invalid_argument("Invalid or excessive Scripture range");
                for (int v = start; v <= end; ++v)
                {
                    const auto mapped = bible.verseMappings.find(chapterKey + ":" + std::to_string(v));
                    std::vector<std::array<int, 2>> addresses{{c, v}};
                    if (mapped != bible.verseMappings.end()) addresses = mapped->second;
                    else if (!nativeMapping && book.requiresExplicitMapping)
                        throw std::invalid_argument("No verified Douay–Rheims correspondence for this " + book.name + " passage");
                    if (addresses.empty())
                    {
                        if (!bible.omittedVerses.contains(chapterKey + ":" + std::to_string(v)))
                            throw std::invalid_argument("Verse mapping is unavailable");
                        result.omitsAbsentVerses = true;
                        continue;
                    }
                    if (addresses.size() != 1 || addresses.front() != std::array<int, 2>{c, v})
                        result.numberingNote = "Whole Douay–Rheims verses shown where numbering or verse divisions differ.";
                    for (const auto& address : addresses)
                    {
                        const auto chapterFound = book.chapters.find(address[0]);
                        if (chapterFound == book.chapters.end()) throw std::invalid_argument("Mapped chapter is absent from the offline Bible");
                        const auto verse = chapterFound->second.find(address[1]);
                        if (verse == chapterFound->second.end()) throw std::invalid_argument("Mapped verse is absent from the offline Bible");
                        const bool boundaryCut = (c == first.chapter && v == first.verse && first.partial) ||
                                                 (c == last.chapter && v == last.verse && last.partial);
                        const bool complete = !nativeMapping && !boundaryCut && addresses.size() == 1 &&
                                              !bible.nonIdentityTargets.contains(chapterKey + ":" + std::to_string(v)) &&
                                              address == std::array<int, 2>{c, v};
                        if (seen.emplace(address[0], address[1]).second)
                            result.verses.push_back({address[0], address[1], verse->second, complete, c, v});
                        else if (complete)
                        {
                            // A separately cited whole verse can make a prior
                            // partial occurrence safe; two fragments cannot.
                            for (auto& existing : result.verses)
                                if (existing.chapter == address[0] && existing.verse == address[1])
                                    existing.completeSourceVerse = true;
                        }
                    }
                }
            }
            trimLeft(text);
            if (!text.empty())
            {
                if (text.front() != ',' && text.front() != '.' && text.front() != ';')
                    throw std::invalid_argument("Unsupported Scripture reference syntax");
                text.erase(0, 1);
                trimLeft(text);
                if (text.empty()) throw std::invalid_argument("Incomplete Scripture reference");
            }
        }
        if (result.omitsAbsentVerses)
            result.numberingNote += (result.numberingNote.empty() ? "" : " ") +
                std::string{"Cited verses absent from the Douay–Rheims text are omitted."};
        if (result.verses.empty()) throw std::invalid_argument("No verses were selected");
        result.bookId = alias->second;
    }
    catch (const std::invalid_argument& error)
    {
        // Discard any verses already collected: a failed passage must not look
        // like a successfully retrieved but shorter reading.
        result = {};
        result.diagnostic = error.what();
    }
    catch (const std::out_of_range&)
    {
        result = {};
        result.diagnostic = "Scripture reference is outside the supported range";
    }
    return result;
}
}

ScripturePassage lookupScripture(const Bible& bible, std::string_view reference, std::string_view psalmNumbering)
{
    return lookupPassage(bible, reference, psalmNumbering, false);
}
