#include "scripture.h"
#include "lectionary_loader.h"
#include "lectionary_resolver.h"
#include "calendar_selection.h"
#include "liturgical_resolver.h"
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
                                "Ps 50:3-4", "John 1:1 7", "John 1:1 rubbish", "Sirach 3:19", "Tobit 11:5-17", "Tobit 12:1"})
        {
            const auto result = lookupScripture(bible, ref);
            require(!result.diagnostic.empty() && result.verses.empty(), "Unsupported lookup fails atomically with a diagnostic");
        }
        require(!lookupScripture(bible, "Ps 98:1–3", "unknown").diagnostic.empty(), "Invalid numbering convention rejected");
        bool missing = false;
        try { loadBible("/tmp/cathfetch-no-such-bible.json"); } catch (const std::exception&) { missing = true; }
        require(missing, "Missing index is reported");

        // Literal passage checks cover changes in text identity, rather than
        // merely accepting any in-range chapter/verse returned by the map.
        const auto charity = lookupScripture(bible, "Tobit 1:3; 2:1–8");
        require(charity.diagnostic.empty() && charity.verses.size() == 10 &&
                charity.verses[0].chapter == 1 && charity.verses[0].verse == 2 &&
                charity.verses[0].text.find("way of truth") != std::string::npos &&
                charity.verses[1].text.starts_with("But every day gave"),
                "Tobit's fidelity and charity require both native introductory verses");
        require(charity.verses.back().chapter == 2 && charity.verses.back().verse == 8 &&
                charity.verses.back().text.find("bury the dead") != std::string::npos &&
                !charity.numberingNote.empty(), "Tobit burial narrative and native numbering notice");
        const auto raphael = lookupScripture(bible, "Tobit 12:1. 5–15. 20");
        require(raphael.diagnostic.empty() && raphael.verses.size() == 13 &&
                raphael.verses[11].verse == 15 &&
                raphael.verses[11].text.starts_with("For I am the angel Raphael") &&
                raphael.verses.back().verse == 20 &&
                raphael.verses.back().text.find("publish all his wonderful works") != std::string::npos,
                "Whole Raphael passage retains revelation and departure without importing intervening verses");
        const std::set<std::string> guardedReferences{
            "Tobit 6:10–11; 7:1. 9–14; 8:4–9", "Tobit 11:5–17",
            "Tobit 13:2. 6–8 r.1", "Tb 13:2-8"};
        for (const auto& ref : guardedReferences)
        {
            const auto passage = lookupScripture(bible, ref);
            require(passage.verses.empty() && passage.diagnostic.find(": ") != std::string::npos,
                    "Reviewed textual differences retain an explanatory atomic failure");
        }
        const auto mercy = lookupScripture(bible, "Ps 51:3-4", "hebrew");
        require(mercy.verses.size() == 2 && mercy.verses[0].chapter == 50 &&
                mercy.verses[0].text.starts_with("Have mercy on me"), "Hebrew psalm titles retain verse alignment");
        const auto bethlehem = lookupScripture(bible, "Micah 5:1-4");
        require(bethlehem.verses.size() == 4 && bethlehem.verses.front().verse == 2 &&
                bethlehem.verses.front().text.starts_with("AND THOU, BETHLEHEM"), "Micah uses Hebrew verse numbering");
        const auto light = lookupScripture(bible, "Isaiah 9:1");
        require(light.verses.size() == 1 && light.verses.front().verse == 2 &&
                light.verses.front().text.starts_with("The people that walked in darkness"), "Isaiah verse shift");
        const auto sirach = lookupScripture(bible, "Sirach 3:2-6");
        require(sirach.verses.size() == 5 && sirach.verses.front().verse == 3 &&
                sirach.verses.front().text.starts_with("For God hath made the father honourable"), "Sirach Greek-to-Latin mapping");
        const auto split = lookupScripture(bible, "Ps 116:1-19", "hebrew");
        require(split.verses.size() == 19 && split.verses.front().chapter == 114 &&
                split.verses.back().chapter == 115, "Hebrew psalm spans two Douay psalms");
        const auto chalice = lookupScripture(bible, "Ps 115:12-13, 15-18 r.13", "lxx");
        require(chalice.verses.size() == 6 && chalice.verses.front().verse == 3 &&
                chalice.verses[1].text.starts_with("I will take the chalice"), "Legacy unsplit psalm verses map explicitly");
        const auto jerusalem = lookupScripture(bible, "Ps 147:12-15, 19-20 r. 12", "lxx");
        require(jerusalem.verses.size() == 6 && jerusalem.verses.front().verse == 1 &&
                jerusalem.verses.front().text.starts_with("Praise the Lord, O Jerusalem"), "Second part of Psalm 147");
        const auto hebrew115 = lookupScripture(bible, "Ps 115:12-13, 15-18 r.13", "hebrew");
        require(hebrew115.verses.front().chapter == 113, "Legacy LXX overrides never affect Hebrew citations");
        require(lookupScripture(bible, "Ps 116(115):10-19").verses.size() == 10,
                "Dual numbering handles split psalms");
        const auto combined = lookupScripture(bible, "Ps 113B:1-4", "lxx");
        require(combined.verses.size() == 4 && combined.verses.front().verse == 9 &&
                combined.verses.front().text.starts_with("Not to us, O Lord"), "Lettered psalm section");
        const auto salt = lookupScripture(bible, "Mark 9:41-50");
        require(salt.verses.size() == 10 && salt.verses.back().verse == 49 &&
                salt.verses.back().text.starts_with("Salt is good"), "Mark source chapter extends beyond target chapter");
        const auto comfort = lookupScripture(bible, "1 Thess 4:13-18");
        require(comfort.verses.size() == 6 && comfort.verses.front().verse == 12 &&
                comfort.verses.back().text.starts_with("Wherefore, comfort"), "Thessalonians merged verse alignment");
        const auto humble = lookupScripture(bible, "Sirach 3:17-20.28-29");
        require(humble.omitsAbsentVerses && humble.diagnostic.empty() &&
                humble.numberingNote.find("absent") != std::string::npos, "Absent source verse is explicitly reported");
        require(!lookupScripture(bible, "Sirach 26:20").diagnostic.empty(), "Absent-only passage returns no unrelated verse");
        require(lookupScripture(bible, "Ps 22 r.1", "lxx").verses.size() == 6, "Whole psalm citation");
        require(lookupScripture(bible, "Phlm 7-20").verses.size() == 14, "Single-chapter letter");
        require(lookupScripture(bible, "Ps 109(110):1b–e.2.3. r.4b", "lxx").includesWholeVerses,
                "Subverse shorthand and publisher punctuation");

        int available = 0, unavailable = 0, omitted = 0, alternatives = 0;
        const auto lectionary = loadLectionary();
        std::map<std::pair<std::string, std::string>, bool> availability;
        auto checkPassage = [&](const std::string& reference, const std::string& numbering)
        {
            const auto passage = lookupScripture(bible, reference, numbering);
            if (!passage.diagnostic.empty())
            {
                require(guardedReferences.contains(reference) &&
                        passage.diagnostic.find("No verified Douay") != std::string::npos && passage.verses.empty(),
                        "Only the documented unverified Tobit correspondences may fail");
                availability[{reference, numbering}] = false;
                return false;
            }
            require(!passage.verses.empty(), "Stored reading returns text");
            availability[{reference, numbering}] = true;
            return passage.omitsAbsentVerses;
        };
        for (const auto& set : lectionary.entries)
            for (const auto& reading : set.readings)
            {
                if (reading.role == "acclamation") continue;
                omitted += checkPassage(reading.reference, reading.psalmNumbering);
                if (availability.at({reading.reference, reading.psalmNumbering})) ++available;
                else ++unavailable;
                for (const auto& alternative : reading.alternatives)
                {
                    checkPassage(alternative.reference, reading.psalmNumbering);
                    require(availability.at({alternative.reference, reading.psalmNumbering}),
                            "Stored alternative reading must have an available text correspondence");
                    ++alternatives;
                }
            }
        require(available == 2661 && unavailable == 4 && omitted == 2,
                "Primary citations resolve except four reviewed Tobit differences; two absent-verse notices");
        int dates = 0, selected = 0, textUnavailableDays = 0;
        using namespace std::chrono;
        for (const auto calendar : {"grc", "europe", "england", "wales"})
        {
            const auto celebrations = loadSelectedCalendar(calendar);
            for (int y = 2024; y <= 2030; ++y)
                for (sys_days date{year{y}/January/1}; date < sys_days{year{y+1}/January/1}; date += days{1})
                {
                    ++dates;
                    const auto day = resolveLiturgicalDay(year_month_day{date}, celebrations, calendar);
                    const auto readings = resolveMassReadings(day, lectionary, calendar);
                    if (!readings.selected) continue; // Explicitly missing assignments remain a separate concern.
                    ++selected;
                    bool textUnavailable = false;
                    for (const auto& reading : readings.selected->readings)
                        if (reading.role != "acclamation")
                        {
                            require(availability.contains({reading.reference, reading.psalmNumbering}),
                                    "Every selected daily passage has a tested text mapping or documented failure");
                            textUnavailable |= !availability.at({reading.reference, reading.psalmNumbering});
                        }
                    textUnavailableDays += textUnavailable;
                }
        }
        std::cout << "Scripture year sweep: " << dates << " dates; " << selected << " selected assignments; "
                  << alternatives << " alternative citations checked; " << omitted << " absent-verse notices; "
                  << textUnavailableDays << " dates with guarded text correspondences\n";
        std::cout << "Scripture: " << checks << " checks; " << verses << " source verses; "
                  << available << " stored citations available, " << unavailable << " explicitly unavailable\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
