// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "calendar_loader.h"
#include "liturgical_precedence.h"

#include <fstream>
#include <set>
#include <stdexcept>
#include <nlohmann/json.hpp>

void updateCelebrationMetadata(FixedCelebration& celebration,
                               const nlohmann::json& changes,
                               const std::string& sourceCalendar)
{
    if (changes.contains("rank"))
    {
        // A regional rank can also change the celebration's precedence class.
        // Keep the rank's origin, not just its displayed name.
        celebration.rank = changes.at("rank").is_null()
            ? std::nullopt : std::optional{parseCelebrationRank(changes.at("rank").get<std::string>())};
        celebration.rankSourceCalendar = sourceCalendar;
    }
    if (changes.contains("name")) celebration.name = changes.at("name");
    if (changes.contains("feast_of_lord")) celebration.feastOfLord = changes.at("feast_of_lord");
    if (changes.contains("principal_patron")) celebration.principalPatron = changes.at("principal_patron");
    if (changes.contains("subject"))
    {
        const std::string subject = changes.at("subject");
        if (subject == "lord") celebration.subject = CelebrationSubject::Lord;
        else if (subject == "saint") celebration.subject = CelebrationSubject::Saint;
        else if (subject == "other") celebration.subject = CelebrationSubject::Other;
        else throw std::invalid_argument("Unknown celebration subject: " + subject);
    }
    if (changes.contains("precedence"))
    {
        const int level = changes.at("precedence");
        if (level < 1 || level > 13) throw std::invalid_argument("Invalid precedence: " + celebration.id);
        celebration.precedence = static_cast<PrecedenceLevel>(level);
    }
    if (changes.contains("names_temporal_baseline"))
        celebration.namesTemporalBaseline = changes.at("names_temporal_baseline");
    if (changes.contains("date_rule"))
    {
        const std::string rule = changes.at("date_rule");
        if (rule == "epiphany") celebration.dateRule = FixedDateRule::Epiphany;
        else if (rule == "all_souls") celebration.dateRule = FixedDateRule::AllSouls;
        else if (rule == "adjacent_sunday") celebration.dateRule = FixedDateRule::AdjacentSunday;
        else if (rule == "nominal") celebration.dateRule = FixedDateRule::Nominal;
        else throw std::invalid_argument("Unknown date rule: " + rule);
    }
    if (changes.contains("transfer_rule"))
    {
        const std::string rule = changes.at("transfer_rule");
        if (rule == "saint_joseph") celebration.transferRule = SolemnityTransferRule::SaintJoseph;
        else if (rule == "annunciation") celebration.transferRule = SolemnityTransferRule::Annunciation;
        else if (rule == "after_easter_octave") celebration.transferRule = SolemnityTransferRule::AfterEasterOctave;
        else if (rule == "before_sacred_heart" || rule == "anticipate_before_lord") celebration.transferRule = SolemnityTransferRule::AnticipateBeforeLord;
        else if (rule == "general") celebration.transferRule = SolemnityTransferRule::General;
        else throw std::invalid_argument("Unknown transfer rule: " + rule);
    }
    if (changes.contains("colour"))
    {
        const std::string colour = changes.at("colour");
        if (colour == "white") celebration.colour = LiturgicalColour::White;
        else if (colour == "red") celebration.colour = LiturgicalColour::Red;
        else if (colour == "violet") celebration.colour = LiturgicalColour::Violet;
        else throw std::invalid_argument("Unknown celebration colour: " + colour);
    }
}

FixedCelebration parseFixedCelebration(const nlohmann::json& entry,
                                      const std::string& sourceCalendar)
{
    const int month = entry.at("month");
    const int day = entry.at("day");
    if (month < 1 || month > 12 || day < 1 || day > 31)
        throw std::invalid_argument("Invalid fixed celebration date");
    FixedCelebration result;
    result.id = entry.at("id");
    result.name = entry.at("name");
    result.date = std::chrono::month{unsigned(month)} / std::chrono::day{unsigned(day)};
    (void)entry.at("rank"); // Required even for a special observance without ordinary rank.
    updateCelebrationMetadata(result, entry, sourceCalendar);
    return result;
}

void validateCalendar(const std::vector<FixedCelebration>& celebrations)
{
    std::set<std::string> ids;
    for (const auto& celebration : celebrations)
    {
        if (celebration.id.empty() || celebration.name.empty() || !celebration.date.ok())
            throw std::invalid_argument("Invalid calendar entry: " + celebration.id);
        if (!ids.insert(celebration.id).second)
            throw std::invalid_argument("Duplicate calendar celebration ID: " + celebration.id);
        fixedCelebrationPrecedence(celebration);
    }
}

std::vector<FixedCelebration> loadFixedCelebrations(const std::string& filename)
{
    std::ifstream file{filename};
    if (!file) throw std::runtime_error("Could not open calendar file: " + filename);
    nlohmann::json calendar;
    file >> calendar;
    std::vector<FixedCelebration> celebrations;
    for (const auto& entry : calendar.at("entries"))
        celebrations.push_back(parseFixedCelebration(entry, calendar.at("calendar")));
    validateCalendar(celebrations);
    return celebrations;
}
