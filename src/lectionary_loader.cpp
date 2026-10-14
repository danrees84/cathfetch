#include "lectionary_loader.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <fstream>
#include <set>
#include <stdexcept>
#include <utility>

namespace
{
bool nonBlank(const std::string& text)
{
    return text.find_first_not_of(" \t\r\n") != std::string::npos;
}
bool sourceUrl(const std::string& text)
{
    return text.starts_with("https://") && text.size() > 8;
}
bool citation(const std::string& text)
{
    return nonBlank(text) && text.size() <= 300 &&
        std::ranges::any_of(text, [](unsigned char c) { return c >= '0' && c <= '9'; }) &&
        std::ranges::none_of(text, [](unsigned char c) { return c < 32 || c == 127; });
}
void validateCalendars(const std::vector<std::string>& calendars)
{
    std::set<std::string> seen;
    if (calendars.empty()) throw std::invalid_argument("Reading assignment needs explicit calendars");
    for (const auto& name : calendars)
        if ((name != "grc" && name != "europe" && name != "england" && name != "wales") ||
            !seen.insert(name).second)
            throw std::invalid_argument("Invalid or duplicate lectionary calendar: " + name);
}
ReadingUse parseUse(const std::string& text)
{
    if (text == "proper") return ReadingUse::Proper;
    if (text == "accommodated") return ReadingUse::Accommodated;
    if (text == "weekday") return ReadingUse::Weekday;
    throw std::invalid_argument("Unknown reading use: " + text);
}
}

void validateLectionary(const Lectionary& data)
{
    if (!nonBlank(data.edition) || !nonBlank(data.referenceConvention))
        throw std::invalid_argument("Lectionary edition and reference convention are required");
    std::set<std::string> ids;
    for (const auto& entry : data.entries)
    {
        if (!nonBlank(entry.id) || !ids.insert(entry.id).second || !nonBlank(entry.dayId) ||
            !sourceUrl(entry.source) || !nonBlank(entry.sourceNote))
            throw std::invalid_argument("Invalid reading identity or provenance: " + entry.id);
        if (entry.cycle != "any" && entry.cycle != "A" && entry.cycle != "B" &&
            entry.cycle != "C" && entry.cycle != "I" && entry.cycle != "II")
            throw std::invalid_argument("Unknown reading cycle: " + entry.cycle);
        if (!isMassForm(entry.mass))
            throw std::invalid_argument("Unknown Mass form: " + entry.mass);
        if (entry.dayType != "any" && entry.dayType != "sunday" && entry.dayType != "weekday")
            throw std::invalid_argument("Unknown reading day type: " + entry.dayType);
        if (entry.lectionaryNumber && *entry.lectionaryNumber <= 0)
            throw std::invalid_argument("Invalid Lectionary number");
        if (entry.partial && entry.use != ReadingUse::Proper)
            throw std::invalid_argument("Only proper readings may replace weekday slots");
        validateCalendars(entry.calendars);
        if (entry.verification == ReferenceVerification::CurrentEdition &&
            (entry.verificationSources.empty() || !nonBlank(entry.verificationNote)))
            throw std::invalid_argument("Current-edition verification requires evidence and scope: " + entry.id);
        for (const auto& source : entry.verificationSources)
            if (!sourceUrl(source)) throw std::invalid_argument("Invalid verification source: " + entry.id);
        std::set<std::string> roles;
        for (const auto& reading : entry.readings)
        {
            if (reading.psalmNumbering != "unspecified" && reading.psalmNumbering != "lxx" &&
                reading.psalmNumbering != "hebrew")
                throw std::invalid_argument("Unknown psalm numbering: " + entry.id);
            const auto& role = reading.role;
            const bool recognised = role == "first" || role == "psalm" || role == "second" ||
                role == "acclamation" || role == "gospel" || role == "procession_gospel" ||
                role == "epistle" || (role.starts_with("reading_") && role.size() == 9 &&
                role.back() >= '1' && role.back() <= '7') ||
                (role.starts_with("psalm_") && role.size() == 7 && role.back() >= '1' && role.back() <= '7');
            if (!recognised || !roles.insert(role).second || !citation(reading.reference))
                throw std::invalid_argument("Invalid or duplicate reading role: " + entry.id + "/" + role);
            std::set<std::string> references{reading.reference};
            for (const auto& option : reading.alternatives)
                if (!citation(option.reference) || !nonBlank(option.condition) ||
                    !references.insert(option.reference).second)
                    throw std::invalid_argument("Invalid reading alternative: " + entry.id);
        }
        if (roles.empty() || (!entry.partial && (!roles.contains("gospel") ||
            (!roles.contains("first") && !roles.contains("reading_1")) ||
            (!roles.contains("psalm") && !roles.contains("psalm_1")))))
            throw std::invalid_argument("Incomplete reading set: " + entry.id);
    }
    for (std::size_t i = 0; i < data.entries.size(); ++i)
        for (std::size_t j = i + 1; j < data.entries.size(); ++j)
        {
            const auto& a = data.entries[i];
            const auto& b = data.entries[j];
            const auto sundayCycle = [](const std::string& cycle) {
                return cycle == "A" || cycle == "B" || cycle == "C";
            };
            const bool cycleOverlaps = a.cycle == b.cycle || a.cycle == "any" || b.cycle == "any" ||
                sundayCycle(a.cycle) != sundayCycle(b.cycle);
            const bool dayOverlaps = a.dayType == b.dayType || a.dayType == "any" || b.dayType == "any";
            if (!a.alternativeOnly && !b.alternativeOnly &&
                a.dayId == b.dayId && a.mass == b.mass && cycleOverlaps && dayOverlaps &&
                std::ranges::any_of(a.calendars, [&](const auto& name) {
                    return std::ranges::find(b.calendars, name) != b.calendars.end();
                }))
                throw std::invalid_argument("Conflicting reading assignments: " + a.dayId);
        }
    std::set<std::pair<std::string, std::string>> policies;
    for (const auto& policy : data.memorials)
    {
        if (!nonBlank(policy.observanceId) || !sourceUrl(policy.source))
            throw std::invalid_argument("Invalid memorial reading policy");
        validateCalendars(policy.calendars);
        for (const auto& calendar : policy.calendars)
            if (!policies.emplace(policy.observanceId, calendar).second)
                throw std::invalid_argument("Conflicting memorial reading policies");
    }
}

Lectionary parseLectionary(const nlohmann::json& document)
{
    if (!document.at("schema_version").is_number_integer() || document.at("schema_version") != 1)
        throw std::invalid_argument("Unsupported lectionary schema");
    Lectionary data;
    data.edition = document.at("edition").get<std::string>();
    data.referenceConvention = document.at("reference_convention").get<std::string>();
    if (!document.at("entries").is_array() || !document.at("memorials").is_array())
        throw std::invalid_argument("Lectionary entries and memorials must be arrays");
    for (const auto& value : document.at("entries"))
    {
        ReadingSet entry;
        entry.id = value.at("id"); entry.dayId = value.at("day_id");
        entry.cycle = value.at("cycle"); entry.mass = value.at("mass");
        entry.dayType = value.value("day_type", "any");
        entry.calendars = value.at("calendars").get<std::vector<std::string>>();
        if (value.contains("lectionary_number") && !value.at("lectionary_number").is_null())
        {
            if (!value.at("lectionary_number").is_number_integer())
                throw std::invalid_argument("Lectionary number must be an integer");
            entry.lectionaryNumber = value.at("lectionary_number").get<int>();
        }
        entry.use = parseUse(value.at("use"));
        entry.partial = value.value("partial", false);
        entry.alternativeOnly = value.value("alternative_only", false);
        entry.permittedAlternative = value.value("permitted_alternative", false);
        const std::string verification = value.at("verification");
        if (verification == "published_table") entry.verification = ReferenceVerification::PublishedTable;
        else if (verification == "current_edition") entry.verification = ReferenceVerification::CurrentEdition;
        else throw std::invalid_argument("Unknown reading-reference verification status");
        entry.verificationSources = value.value("verification_sources", std::vector<std::string>{});
        entry.verificationNote = value.value("verification_note", std::string{});
        entry.source = value.at("source"); entry.sourceNote = value.at("source_note");
        if (!value.at("readings").is_array()) throw std::invalid_argument("Readings must be an array");
        for (const auto& item : value.at("readings"))
        {
            ReadingReference reading{item.at("role"), item.at("reference"), {}};
            reading.psalmNumbering = item.value("psalm_numbering", std::string{"unspecified"});
            if (item.contains("alternatives"))
            {
                if (!item.at("alternatives").is_array()) throw std::invalid_argument("Alternatives must be an array");
                for (const auto& option : item.at("alternatives"))
                    reading.alternatives.push_back({option.at("reference"), option.at("condition")});
            }
            entry.readings.push_back(std::move(reading));
        }
        data.entries.push_back(std::move(entry));
    }
    for (const auto& value : document.at("memorials"))
        data.memorials.push_back({value.at("observance_id"),
            value.at("calendars").get<std::vector<std::string>>(), parseUse(value.at("use")), value.at("source")});
    validateLectionary(data);
    return data;
}

Lectionary loadLectionary(const std::string& filename)
{
    std::ifstream file{filename};
    if (!file) throw std::runtime_error("Could not open lectionary file: " + filename);
    try
    {
        nlohmann::json document;
        file >> document;
        return parseLectionary(document);
    }
    catch (const std::exception& error)
    {
        throw std::runtime_error(filename + ": " + error.what());
    }
}
