#include "scripture.h"
#include "lectionary_loader.h"
#include <iostream>
#include <stdexcept>

namespace
{
int checks = 0;
void require(bool value, const char* message)
{
    ++checks;
    if (!value) throw std::runtime_error(message);
}
}

int main()
{
    try
    {
        const auto bible = loadBible();
        require(bible.books.size() == 73, "Complete Catholic canon is bundled");
        std::size_t verses = 0;
        for (const auto& [id, book] : bible.books)
            for (const auto& [chapter, contents] : book.chapters) verses += contents.size();
        require(verses == 35811, "Pinned source verse count");
        for (const auto& id : {"TOB", "JDT", "WIS", "SIR", "BAR", "1MA", "2MA"})
            require(bible.books.contains(id), "Deuterocanonical book is present");
        const auto john = lookupScripture(bible, "John 1:1-18");
        require(john.diagnostic.empty() && john.verses.size() == 18, "Christmas Gospel");
        require(john.verses[0].text == "In the beginning was the Word, and the Word was with God, and the Word was God.",
                "Independent well-known Douay–Rheims wording");
        const auto isaiah = lookupScripture(bible, "Isa 52:7-10");
        require(isaiah.verses.size() == 4 && isaiah.verses.front().verse == 7, "Christmas first reading");
        const auto crossChapter = lookupScripture(bible, "Genesis 1:31–2:3");
        require(crossChapter.verses.size() == 4 && crossChapter.verses.back().chapter == 2,
                "Cross-chapter range preserves chapter boundaries");
        const auto separated = lookupScripture(bible, "John 1:1-5, 9-14");
        require(separated.verses.size() == 11 && separated.verses[5].verse == 9,
                "Discontinuous passage omits intervening verses");
        const auto semicolon = lookupScripture(bible, "Acts 11:21b–26; 13:1–3");
        require(semicolon.verses.size() == 9 && semicolon.includesWholeVerses,
                "Semicolon chapter change and partial endpoint");
        const auto partial = lookupScripture(bible, "John 1:1a, 1b–2");
        require(partial.verses.size() == 2 && partial.includesWholeVerses, "Partial selections do not duplicate whole verses");
        require(lookupScripture(bible, "1Cor 1:1–3").verses.size() == 3, "Numbered book without space");
        require(lookupScripture(bible, "I John 3:1–3").verses.size() == 3, "Roman book alias");
        require(lookupScripture(bible, "Ps Lk 1:46–55").verses.size() == 10, "Canticle label resolves the actual book");
        const auto psalm = lookupScripture(bible, "Ps 97:1, 2-3a, 3b-4, 5-6", "lxx");
        require(psalm.verses.size() == 6 && psalm.includesWholeVerses, "Christmas psalm coalesces overlapping subverses");
        require(lookupScripture(bible, "Psalm 98(97):1–6").verses.size() == 6, "Explicit dual psalm numbering");
        require(lookupScripture(bible, "Psalm 97(98):1–6").verses.size() == 6, "Reverse dual psalm numbering");
        require(lookupScripture(bible, "Psalm 117:1–2", "hebrew").verses.front().chapter == 116,
                "Verified Hebrew-to-Douay psalm chapter mapping");
        const auto praise = lookupScripture(bible, "Psalm 100(99):1–3, 5");
        require(praise.verses.size() == 3 && praise.verses[0].verse == 2 && !praise.numberingNote.empty(),
                "Psalm 100 requires a verse map, not just a chapter shift");
        require(praise.verses.front().text.starts_with("Sing joyfully"), "Psalm mapping excludes a title-only verse");
        const auto daniel = lookupScripture(bible, "Dan 3:52-56 r. 52");
        require(daniel.verses.size() == 5 && daniel.verses[0].text.starts_with("Blessed art thou"),
                "Catholic Daniel canticle is present and mapped");
        const auto furnace = lookupScripture(bible, "Daniel 3:14-20. 24-25. 28");
        require(furnace.verses.size() == 10 && furnace.verses[7].verse == 91,
                "Modern Daniel verses map around the inserted Greek prayer");
        require(lookupScripture(bible, "Wisdom 3:1–9").verses.size() == 9, "Deuterocanonical reading text");
        for (const auto& ref : {"John 1:0", "John 1:999", "John 999:1", "John 1:18-1", "John 1:1,",
                                "John 1:1 or 2:1", "Unknown 1:1", "John 1:99999999999999999",
                                "Ps 50:3-4", "Tobit 1:1", "Isa 52:7; 9:1"})
        {
            const auto result = lookupScripture(bible, ref);
            require(!result.diagnostic.empty() && result.verses.empty(), "Unsupported lookup fails atomically with a diagnostic");
        }
        require(!lookupScripture(bible, "Ps 98:1–3", "unknown").diagnostic.empty(), "Invalid numbering convention rejected");
        bool missing = false;
        try { loadBible("/tmp/cathfetch-no-such-bible.json"); } catch (const std::exception&) { missing = true; }
        require(missing, "Missing index is reported");

        int available = 0, unavailable = 0;
        for (const auto& set : loadLectionary().entries)
            for (const auto& reading : set.readings)
            {
                if (reading.role == "acclamation") continue;
                const auto passage = lookupScripture(bible, reading.reference, reading.psalmNumbering);
                require(passage.diagnostic.empty() == !passage.verses.empty(), "Every stored citation produces text or an explicit failure");
                if (passage.diagnostic.empty()) ++available;
                else ++unavailable;
            }
        std::cout << "Scripture: " << checks << " checks; " << verses << " source verses; "
                  << available << " stored citations available, " << unavailable << " explicitly unavailable\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
