#ifndef SCRIPTURE_H
#define SCRIPTURE_H

#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

struct BibleBook
{
    std::string name;
    std::map<int, std::map<int, std::string>> chapters;
    bool requiresMapping = false;
    std::set<int> unmappedChapters;
    std::map<int, std::set<int>> unmappedVerses;
};

struct Bible
{
    std::string translation;
    std::map<std::string, BibleBook> books;
    std::map<std::string, std::string> aliases;
    std::set<int> verifiedPsalms;
    std::map<std::string, std::string> passageMappings;
    std::map<std::string, std::map<int, int>> psalmVerseMappings;
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
    std::string numberingNote;
    std::string diagnostic; // Nonempty means no text is returned for this passage.
};

Bible loadBible(const std::string& filename = "build/en_dra.json");
// psalmNumbering: unspecified, lxx or hebrew. Explicit dual numbering is also
// recognised. Unverified conversions fail; chapter shifts alone are insufficient.
ScripturePassage lookupScripture(const Bible& bible, std::string_view reference,
                                std::string_view psalmNumbering = "unspecified");

#endif
