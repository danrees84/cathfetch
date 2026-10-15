// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

// Authoring aid: expose the engine's checked verse candidates, not a second
// reference parser. Nothing in this program selects a verse editorially.
#include "lectionary_loader.h"
#include "scripture.h"
#include <nlohmann/json.hpp>
#include <iostream>

namespace
{
nlohmann::json eligibleVerses(const ScripturePassage& passage)
{
    auto verses = nlohmann::json::array();
    for (const auto& verse : passage.verses)
        if (verse.completeSourceVerse)
            verses.push_back({{"book", passage.bookId}, {"chapter", verse.chapter},
                              {"verse", verse.verse}, {"quotation", verse.text}});
    return verses;
}
}

int main()
{
    const auto bible = loadBible();
    nlohmann::json catalogue = nlohmann::json::array();
    for (const auto& set : loadLectionary().entries)
    {
        nlohmann::json item{{"id", set.id}, {"readings", nlohmann::json::array()}};
        for (const auto& reading : set.readings)
        {
            if (reading.role == "acclamation") continue;
            const auto passage = lookupScripture(bible, reading.reference, reading.psalmNumbering);
            auto alternatives = nlohmann::json::array();
            for (const auto& option : reading.alternatives)
            {
                const auto variant = lookupScripture(bible, option.reference, reading.psalmNumbering);
                alternatives.push_back({{"reference", option.reference}, {"condition", option.condition},
                                        {"candidates", eligibleVerses(variant)}, {"diagnostic", variant.diagnostic}});
            }
            item["readings"].push_back({{"role", reading.role}, {"reference", reading.reference},
                                         {"candidates", eligibleVerses(passage)}, {"diagnostic", passage.diagnostic},
                                         {"alternatives", alternatives}});
        }
        catalogue.push_back(item);
    }
    std::cout << catalogue.dump(2) << '\n';
}
