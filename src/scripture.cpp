#include "scripture.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <stdexcept>

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

int vulgatePsalm(int hebrew)
{
    if (hebrew < 1 || hebrew > 150) throw std::invalid_argument("Invalid psalm number");
    if (hebrew <= 9 || hebrew >= 148) return hebrew;
    if (hebrew <= 113) return hebrew - 1;
    if (hebrew <= 115) return 113;
    if (hebrew == 116) return -1;
    if (hebrew <= 146) return hebrew - 1;
    return -1;
}

struct Address { int chapter; int verse; bool partial; };

Address endpoint(std::string& text, int chapter)
{
    static const std::regex pattern{R"(^\s*([0-9]+)(?:\s*:\s*([0-9]+))?([a-e]*))"};
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
}

Bible loadBible(const std::string& filename)
{
    std::ifstream file{filename};
    if (!file) throw std::runtime_error("Could not open offline Bible index: " + filename + "; run make all");
    nlohmann::json data;
    file >> data;
    if (data.at("schema_version") != 1) throw std::invalid_argument("Unsupported Bible index schema");
    Bible bible;
    bible.translation = data.at("translation");
    bible.verifiedPsalms = data.at("verified_psalms").get<std::set<int>>();
    bible.passageMappings = data.value("passage_mappings", std::map<std::string, std::string>{});
    if (data.contains("psalm_verse_mappings"))
        for (const auto& [key, mappings] : data["psalm_verse_mappings"].items())
            for (const auto& [source, target] : mappings.items())
                bible.psalmVerseMappings[key][std::stoi(source)] = target.get<int>();
    for (const auto& [id, entry] : data.at("books").items())
    {
        BibleBook book;
        book.name = entry.at("name");
        if (entry.contains("unmapped_chapters"))
        {
            if (entry["unmapped_chapters"].is_string())
            {
                if (entry["unmapped_chapters"] != "all") throw std::invalid_argument("Invalid Bible mapping policy");
                book.requiresMapping = true;
            }
            else book.unmappedChapters = entry["unmapped_chapters"].get<std::set<int>>();
        }
        if (entry.contains("unmapped_verses"))
            for (const auto& [chapter, verses] : entry["unmapped_verses"].items())
                book.unmappedVerses[std::stoi(chapter)] = verses.get<std::set<int>>();
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
        std::string text{reference};
        for (const auto& dash : {"–", "—", "−"}) replaceAll(text, dash, "-");
        replaceAll(text, " ", " ");
        replaceAll(text, " and ", ", ");
        // The responsorial refrain is not another passage to look up.
        static const std::regex response{R"(\s+[rR]\.\s*|\s*℟)"};
        std::smatch match;
        if (std::regex_search(text, match, response)) text.erase(match.position());
        text = std::regex_replace(text, std::regex{R"(^Ps\s+(?=[A-Za-z]))"}, "");
        static const std::regex heading{R"(^\s*((?:[1-3]\s*)?[A-Za-z]+(?:\s+[A-Za-z]+)*)\s*([0-9]+)(?:\(([0-9]+)\))?\s*:\s*(.*)$)"};
        if (!std::regex_match(text, match, heading)) throw std::invalid_argument("Unsupported Scripture reference syntax");
        const auto alias = bible.aliases.find(aliasKey(match[1].str()));
        if (alias == bible.aliases.end()) throw std::invalid_argument("Book name is not recognised");
        const auto& book = bible.books.at(alias->second);
        const bool psalm = alias->second == "PSA";
        int chapter = std::stoi(match[2]);
        std::string key = alias->second + std::to_string(chapter) + ":" + match[4].str();
        key.erase(std::remove_if(key.begin(), key.end(), [](unsigned char c) { return std::isspace(c); }), key.end());
        replaceAll(key, ".", ",");
        if (!nativeMapping)
            if (const auto mapped = bible.passageMappings.find(key); mapped != bible.passageMappings.end())
            {
                auto passage = lookupPassage(bible, mapped->second, "lxx", true);
                if (passage.diagnostic.empty()) passage.numberingNote = "Douay–Rheims numbering is used below.";
                return passage;
            }
        if (psalmNumbering != "unspecified" && psalmNumbering != "lxx" && psalmNumbering != "hebrew")
            throw std::invalid_argument("Unknown psalm numbering convention");
        if (match[3].matched)
        {
            if (!psalm) throw std::invalid_argument("Dual numbering is supported only for psalms");
            const int other = std::stoi(match[3]);
            // Common H(LXX) and LXX(H) notations are both accepted, only when
            // the pair can be verified. Verse alignment still needs a policy.
            if (vulgatePsalm(chapter) == other) psalmNumbering = "hebrew";
            else if (vulgatePsalm(other) == chapter) psalmNumbering = "lxx";
            else throw std::invalid_argument("Unrecognised dual psalm numbering");
        }
        text = match[4];
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
                last = endpoint(text, chapter);
                chapter = last.chapter;
            }
            if (last.chapter < first.chapter ||
                (last.chapter == first.chapter && last.verse < first.verse) || last.chapter - first.chapter > 150)
                throw std::invalid_argument("Reversed or excessive Scripture range");
            result.includesWholeVerses |= first.partial || last.partial;
            for (int c = first.chapter; c <= last.chapter; ++c)
            {
                int native = c;
                const auto verseMapping = bible.psalmVerseMappings.find(
                    std::string{psalmNumbering} + ":" + std::to_string(c));
                if (psalm)
                {
                    if (psalmNumbering == "hebrew") native = vulgatePsalm(c);
                    else if (psalmNumbering == "unspecified" && c > 2)
                        throw std::invalid_argument("Psalm numbering has not been identified for this reference");
                    if (!nativeMapping && !bible.verifiedPsalms.contains(native) && verseMapping == bible.psalmVerseMappings.end())
                        throw std::invalid_argument("Psalm verse numbering has not yet been mapped to Douay–Rheims");
                    if (native != c) result.numberingNote = "Douay–Rheims psalm numbering is used below.";
                }
                if (!nativeMapping && (book.requiresMapping || book.unmappedChapters.contains(native)))
                    throw std::invalid_argument("Verse numbering for this passage has not yet been mapped to Douay–Rheims");
                const auto found = book.chapters.find(native);
                if (found == book.chapters.end()) throw std::invalid_argument("Chapter is absent from the offline Bible");
                const int start = c == first.chapter ? first.verse : 1;
                const int end = c == last.chapter ? last.verse : found->second.rbegin()->first;
                if (end - start > 2000) throw std::invalid_argument("Excessive Scripture range");
                for (int v = start; v <= end; ++v)
                {
                    if (!nativeMapping)
                        if (const auto guarded = book.unmappedVerses.find(c);
                            guarded != book.unmappedVerses.end() && guarded->second.contains(v))
                            throw std::invalid_argument("Verse numbering for this passage has not yet been mapped to Douay–Rheims");
                    int nativeVerse = v;
                    if (psalm && verseMapping != bible.psalmVerseMappings.end())
                    {
                        const auto mapped = verseMapping->second.find(v);
                        if (mapped == verseMapping->second.end()) throw std::invalid_argument("Psalm verse mapping is unavailable");
                        nativeVerse = mapped->second;
                        if (nativeVerse != v) result.numberingNote = "Whole Douay–Rheims verses shown where verse divisions differ.";
                    }
                    const auto verse = found->second.find(nativeVerse);
                    if (verse == found->second.end()) throw std::invalid_argument("Verse is absent from the offline Bible");
                    if (seen.emplace(native, nativeVerse).second) result.verses.push_back({native, nativeVerse, verse->second});
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
        if (result.verses.empty()) throw std::invalid_argument("No verses were selected");
    }
    catch (const std::invalid_argument& error) { result = {}; result.diagnostic = error.what(); }
    catch (const std::out_of_range&) { result = {}; result.diagnostic = "Scripture reference is outside the supported range"; }
    return result;
}
}

ScripturePassage lookupScripture(const Bible& bible, std::string_view reference, std::string_view psalmNumbering)
{
    return lookupPassage(bible, reference, psalmNumbering, false);
}
