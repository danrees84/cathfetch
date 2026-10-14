#ifndef READING_SET_H
#define READING_SET_H

#include "lectionary_cycle.h"
#include <optional>
#include <string>
#include <string_view>
#include <vector>

inline bool isMassForm(std::string_view name)
{
    for (std::string_view form : {"day", "vigil", "vigil-extended", "night", "dawn", "evening", "passion"})
        if (name == form) return true;
    return false;
}

// Citations only. Neither Scripture text nor liturgical responses belong here.
struct ReadingOption
{
    std::string reference;
    std::string condition;
};

struct ReadingReference
{
    std::string role; // first, psalm, second, acclamation, gospel; numbered at vigils.
    std::string reference;
    std::vector<ReadingOption> alternatives;
};

enum class ReadingUse { Proper, Accommodated, Weekday };
enum class ReferenceVerification { PublishedTable, CurrentEdition };

struct ReadingSet
{
    std::string id;
    std::string dayId; // Resolved observance ID or a structured temporal key.
    std::string cycle; // any, A, B, C, I or II.
    std::string mass;  // See isMassForm; passion is a liturgy, not a Mass.
    std::string dayType = "any"; // any, sunday or weekday (feasts have two readings on weekdays).
    std::vector<std::string> calendars;
    std::optional<int> lectionaryNumber;
    ReadingUse use = ReadingUse::Proper;
    bool partial = false; // Proper memorial reading(s) replacing weekday slots.
    bool alternativeOnly = false; // Permitted set, never the deterministic default.
    bool permittedAlternative = false; // This selected set is itself an optional provision.
    ReferenceVerification verification = ReferenceVerification::PublishedTable;
    std::vector<std::string> verificationSources;
    std::string verificationNote;
    std::vector<ReadingReference> readings;
    std::string source;
    std::string sourceNote;
};

struct MemorialReadingPolicy
{
    std::string observanceId;
    std::vector<std::string> calendars;
    ReadingUse use;
    std::string source;
};

struct Lectionary
{
    std::string edition;
    std::string referenceConvention;
    std::vector<ReadingSet> entries;
    std::vector<MemorialReadingPolicy> memorials;
};

enum class ReadingStatus { Available, MissingData, NoDaytimeMass, UnresolvedDay };

struct LectionaryDay
{
    ReadingCycles cycles;
    ReadingStatus status = ReadingStatus::MissingData;
    std::string key;
    std::optional<ReadingSet> selected;
    std::vector<ReadingSet> alternatives;
    std::vector<std::string> notes;
    std::string diagnostic;
};

#endif
