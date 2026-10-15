#include "lectionary_resolver.h"
#include "easter.h"
#include <algorithm>
#include <stdexcept>

namespace
{
void validateCalendar(std::string_view name)
{
    if (name != "grc" && name != "europe" && name != "england" && name != "wales")
        throw std::invalid_argument("Unknown lectionary calendar: " + std::string{name});
}
bool applies(const std::vector<std::string>& calendars, std::string_view calendar)
{
    return std::ranges::find(calendars, calendar) != calendars.end();
}
// A national record does not imply a universal record. In particular, finding
// no records at all must still report missing data for a universal celebration.
bool hasOnlyNationalAssignments(const Lectionary& data, const std::string& key)
{
    bool foundNational = false;
    for (const auto& entry : data.entries)
    {
        if (entry.dayId != key) continue;
        if (applies(entry.calendars, "grc")) return false;
        foundNational = true;
    }
    return foundNational;
}

std::vector<ReadingSet> matchingAssignments(const Lectionary& data, const std::string& key,
                                           const ReadingCycles& cycles, std::string_view calendar,
                                           std::chrono::year_month_day date)
{
    const bool sunday = std::chrono::weekday{std::chrono::sys_days{date}} == std::chrono::Sunday;
    const std::string_view dayType = sunday ? "sunday" : "weekday";
    std::vector<ReadingSet> matches;
    for (const auto& entry : data.entries)
    {
        if (entry.partial || entry.dayId != key || !applies(entry.calendars, calendar)) continue;
        const bool matchesDayType = entry.dayType == "any" || entry.dayType == dayType;
        const bool matchesCycle = entry.cycle == "any" || entry.cycle == cycleName(cycles.sunday) ||
                                  entry.cycle == cycleName(cycles.weekday);
        if (matchesDayType && matchesCycle)
        {
            matches.push_back(entry);
        }
    }

    // Sorting makes the returned alternatives predictable. It does not decide
    // which reading set wins; two default sets for the same Mass are an error.
    std::ranges::sort(matches, {}, &ReadingSet::id);
    for (std::size_t first = 0; first < matches.size(); ++first)
    {
        for (std::size_t second = first + 1; second < matches.size(); ++second)
        {
            if (!matches[first].alternativeOnly && !matches[second].alternativeOnly &&
                matches[first].mass == matches[second].mass)
            {
                throw std::logic_error("Ambiguous reading assignment: " + key);
            }
        }
    }
    return matches;
}
}

std::string temporalReadingKey(const LiturgicalDay& day, std::string_view calendarName)
{
    using namespace std::chrono;
    validateCalendar(calendarName);
    (void)readingCycles(day.date); // Validate the date before calculating a key.
    const sys_days date{day.date};
    const unsigned weekdayNumber = weekday{date}.c_encoding();
    const auto weekdayKey = std::to_string(weekdayNumber);
    const std::string prefix = "temporal:";
    switch (day.season)
    {
        case LiturgicalPeriod::OrdinaryTime:
            return prefix + "ordinary:" + std::to_string(calculateOrdinaryTimeWeek(day.date, calendarName)) + ":" + weekdayKey;
        case LiturgicalPeriod::Advent:
            if (weekdayNumber != 0 && day.date.month() == December && unsigned(day.date.day()) >= 17)
                return prefix + "advent:december:" + std::to_string(unsigned(day.date.day()));
            return prefix + "advent:" + std::to_string(calculateAdventWeek(day.date)) + ":" + weekdayKey;
        case LiturgicalPeriod::Easter:
        case LiturgicalPeriod::Lent:
        case LiturgicalPeriod::Triduum:
        {
            // Easter is day zero. Negative offsets count backwards: Ash Wednesday
            // is -46, the first Sunday of Lent is -42, and Palm Sunday is -7.
            const auto offset = (date - sys_days{calculateEaster(int(day.date.year()))}).count();
            if (offset == -46) return prefix + "ash_wednesday";
            if (offset >= -45 && offset <= -43) return prefix + "after_ashes:" + weekdayKey;
            if (offset >= -7 && offset <= -1) return prefix + "holy_week:" + weekdayKey;
            if (offset < 0) return prefix + "lent:" + std::to_string((offset + 42) / 7 + 1) + ":" + weekdayKey;
            return prefix + "easter:" + std::to_string(offset / 7 + 1) + ":" + weekdayKey;
        }
        case LiturgicalPeriod::Christmas:
            if (weekdayNumber == 0) return prefix + "christmas:sunday2";
            if (day.date.month() == December)
                return prefix + "christmas:december:" + std::to_string(unsigned(day.date.day()));
            if (day.date <= calculateEpiphany(int(day.date.year()), calendarName))
                return prefix + "christmas:january:" + std::to_string(unsigned(day.date.day()));
            // With a fixed 6 January Epiphany the sequence follows calendar dates;
            // with Sunday Epiphany it follows weekdays (LM Introduction 96).
            return prefix + "after_epiphany:" + std::to_string(
                calculateEpiphany(int(day.date.year()), calendarName).day() == std::chrono::day{6}
                    ? unsigned(day.date.day()) - 6 : weekdayNumber);
    }
    throw std::logic_error("Unknown temporal reading season");
}

LectionaryDay resolveMassReadings(const LiturgicalDay& day, const Lectionary& data,
                                 std::string_view calendarName, std::string_view mass)
{
    validateCalendar(calendarName);
    if (mass != "default" && !isMassForm(mass))
        throw std::invalid_argument("Unknown Mass form: " + std::string{mass});
    LectionaryDay result;
    result.cycles = readingCycles(day.date);
    if (!day.unresolvedReason.empty() || day.observanceId.empty())
    {
        result.status = ReadingStatus::UnresolvedDay;
        result.diagnostic = "The calendar has no resolved observance";
        return result;
    }
    if (day.provisionalTransfer) result.notes.push_back(day.transferNote);
    const auto temporal = temporalReadingKey(day, calendarName);
    result.key = day.observanceId == "temporal" ? temporal : day.observanceId;
    // Holy Saturday has no daytime Mass. The Vigil belongs to Easter Sunday,
    // but is available explicitly for the civil Saturday evening.
    if (temporal == "temporal:holy_week:6")
    {
        if (mass == "default" || mass == "day")
        {
            result.status = ReadingStatus::NoDaytimeMass;
            result.diagnostic = "Holy Saturday has no daytime Mass; Easter Vigil readings are a separate form";
            return result;
        }
        if (mass == "vigil") result.key = "easter_sunday";
    }
    // V1 uses the universal cycle: no memorial policies, Common selections or
    // proper memorial slots. The caller supplies the General Roman day using
    // the selected calendar's scheduling of universal celebrations.
    const bool memorial = day.rank == CelebrationRank::Memorial ||
                          day.rank == CelebrationRank::OptionalMemorial;
    if (memorial || hasOnlyNationalAssignments(data, result.key)) result.key = temporal;
    auto sets = matchingAssignments(data, result.key, result.cycles, "grc", day.date);
    const std::string_view requestedMass = mass == "default" ? "day" : mass;
    auto selected = std::ranges::find_if(sets, [&](const auto& set) {
        return !set.alternativeOnly && set.mass == requestedMass;
    });
    // Thursday evening and Good Friday have a single proper liturgy, not a
    // fictitious daytime Mass. A Vigil is never implicitly selected.
    if (selected == sets.end() && mass == "default" && sets.size() == 1 && !sets.front().alternativeOnly &&
        (sets.front().mass == "evening" || sets.front().mass == "passion"))
        selected = sets.begin();
    if (selected == sets.end())
    {
        // TODO(v2): restore verified national propers, including the Six Welsh
        // Martyrs (25 October), only with an explicit national-reading policy.
        result.alternatives.insert(result.alternatives.end(), sets.begin(), sets.end());
        result.diagnostic = "Reading assignment unavailable for " + result.key +
            " (" + std::string{mass == "default" ? "day" : mass} + ")";
        return result;
    }
    result.selected = *selected;
    for (auto it = sets.begin(); it != sets.end(); ++it)
        if (it != selected) result.alternatives.push_back(*it);
    result.status = ReadingStatus::Available;
    result.notes.push_back("V1 universal reading cycle; memorial and national propers omitted");
    result.notes.push_back(data.referenceConvention);
    result.notes.push_back(result.selected->sourceNote);
    return result;
}
