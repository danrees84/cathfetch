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
std::vector<ReadingSet> assignments(const Lectionary& data, const std::string& key,
                                   const ReadingCycles& cycles, std::string_view calendar,
                                   std::chrono::year_month_day date)
{
    std::vector<ReadingSet> matches;
    for (const auto& entry : data.entries)
        if (!entry.partial && entry.dayId == key && applies(entry.calendars, calendar) &&
            (entry.dayType == "any" || entry.dayType ==
                (std::chrono::weekday{std::chrono::sys_days{date}} == std::chrono::Sunday ? "sunday" : "weekday")) &&
            (entry.cycle == "any" || entry.cycle == cycleName(cycles.sunday) ||
             entry.cycle == cycleName(cycles.weekday)))
            matches.push_back(entry);
    std::ranges::sort(matches, {}, &ReadingSet::id);
    // No implicit specificity or file-order policy: overlapping matches are errors.
    for (std::size_t i = 0; i < matches.size(); ++i)
        for (std::size_t j = i + 1; j < matches.size(); ++j)
            if (!matches[i].alternativeOnly && !matches[j].alternativeOnly && matches[i].mass == matches[j].mass)
                throw std::logic_error("Ambiguous reading assignment: " + key);
    return matches;
}
}

std::string temporalReadingKey(const LiturgicalDay& day, std::string_view calendarName)
{
    using namespace std::chrono;
    validateCalendar(calendarName);
    (void)readingCycles(day.date);
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
    LectionaryDay result{readingCycles(day.date), ReadingStatus::MissingData, {}, {}, {}, {}, {}};
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
    const bool nationalProper = std::ranges::any_of(data.entries, [&](const auto& entry) {
        return entry.dayId == result.key && !applies(entry.calendars, "grc");
    }) && std::ranges::none_of(data.entries, [&](const auto& entry) {
        return entry.dayId == result.key && applies(entry.calendars, "grc");
    });
    if (memorial || nationalProper) result.key = temporal;
    auto sets = assignments(data, result.key, result.cycles, "grc", day.date);
    const auto chosen = std::ranges::find_if(sets, [&](const auto& set) {
        return !set.alternativeOnly && set.mass == (mass == "default" ? "day" : mass);
    });
    // Thursday evening and Good Friday have a single proper liturgy, not a
    // fictitious daytime Mass. A Vigil is never implicitly selected.
    auto selected = chosen;
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
