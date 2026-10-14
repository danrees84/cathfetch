#ifndef SCRIPTURE_H
#define SCRIPTURE_H

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <array>

struct BibleBook
{
    std::string name;
    std::map<int, std::map<int, std::string>> chapters;
    std::string numbering;
    std::set<int> nativeChapters;
};

struct Bible
{
    std::string translation;
    std::map<std::string, BibleBook> books;
    std::map<std::string, std::string> aliases;
    std::map<std::string, std::string> passageMappings;
    std::map<std::string, std::vector<std::array<int, 2>>> verseMappings;
    std::map<std::string, int> chapterEnds;
    std::set<std::string> omittedVerses;
};

struct ScriptureVerse
{
    int chapter;
    int verse;
    std::string text;
};

struct ScripturePassage
{
    std::vector<ScriptureVerse> verses;
    bool includesWholeVerses = false;
    bool omitsAbsentVerses = false;
    std::string numberingNote;
    std::string diagnostic; // Nonempty means no text is returned for this passage.
};

Bible loadBible(const std::string& filename = "build/en_dra.json");
// psalmNumbering: unspecified, lxx or hebrew. Explicit dual numbering is also
// recognised. Unverified conversions fail; chapter shifts alone are insufficient.
ScripturePassage lookupScripture(const Bible& bible, std::string_view reference,
                                std::string_view psalmNumbering = "unspecified");

#endif
