#include "liturgical_resolver.h"
#include "calendar_loader.h"
#include "liturgical_format.h"
#include "movable_observances.h"
#include "easter.h"

#include <algorithm>
#include <stdexcept>

namespace
{
using namespace std::chrono;

ObservanceCandidate temporalCandidate(year_month_day date, std::string_view calendar)
{
    ObservanceCandidate result;
    result.id = "temporal";
    result.precedence = temporalBaselinePrecedence(date, calendar);
    const auto season = getLiturgicalPeriod(date, calendar);
    result.colour = seasonalColour(season);
    switch (season)
    {
        case LiturgicalPeriod::OrdinaryTime:
            result.name = formatOrdinaryTimeDay(date, calculateOrdinaryTimeWeek(date, calendar));
            break;
        case LiturgicalPeriod::Advent:
            result.name = formatAdventDay(date, calculateAdventWeek(date));
            break;
        case LiturgicalPeriod::Christmas:
            result.name = formatChristmasDay(date, calendar);
            break;
        case LiturgicalPeriod::Easter:
            // Named candidates replace these temporal identities below.
            if (date == calculateEaster(int(date.year()))) result.name = "Easter Sunday";
            else if (date == calculatePentecost(int(date.year()))) result.name = "Pentecost Sunday";
            else result.name = formatEasterDay(date);
            break;
        case LiturgicalPeriod::Lent:
        case LiturgicalPeriod::Triduum:
            result.name = formatLentenDay(date);
            break;
    }
    const sys_days day{date};
    const sys_days easter{calculateEaster(int(date.year()))};
    if (day == easter - days{7} || day == easter - days{2}) result.colour = LiturgicalColour::Red;
    if (day == easter - days{3} || day == easter - days{1}) result.colour = std::nullopt;
    return result;
}

year_month_day scheduledDate(const FixedCelebration& celebration, int year,
                             std::string_view calendar)
{
    if (celebration.dateRule == FixedDateRule::Epiphany)
        return calculateEpiphany(year, calendar);
    year_month_day date{std::chrono::year{year} / celebration.date};
    if (celebration.dateRule == FixedDateRule::AdjacentSunday &&
        (calendar == "england" || calendar == "wales"))
    {
        const sys_days day{date};
        if (weekday{day} == Saturday) return year_month_day{day + days{1}};
        if (weekday{day} == Monday) return year_month_day{day - days{1}};
    }
    return date;
}

ObservanceCandidate fixedCandidate(const FixedCelebration& celebration)
{
    ObservanceCandidate candidate;
    candidate.id = celebration.id;
    candidate.name = celebration.name;
    candidate.rank = celebration.rank;
    candidate.precedence = fixedCelebrationPrecedence(celebration);
    candidate.optional = celebration.rank == CelebrationRank::OptionalMemorial;
    candidate.namesTemporalBaseline = celebration.namesTemporalBaseline;
    candidate.colour = celebration.colour;
    return candidate;
}

std::vector<ObservanceCandidate> dayCandidates(year_month_day date,
    const std::vector<FixedCelebration>& celebrations, std::string_view calendar)
{
    auto candidates = findMovableObservances(date, calendar);
    for (const auto& celebration : celebrations)
        if (scheduledDate(celebration, int(date.year()), calendar) == date)
            candidates.push_back(fixedCandidate(celebration));
    return candidates;
}

LiturgicalDay selectDay(year_month_day date, std::vector<ObservanceCandidate> candidates,
                       std::string_view calendar)
{
    auto baseline = temporalCandidate(date, calendar);
    std::optional<ObservanceCandidate> namedBaseline;
    for (const auto& candidate : candidates)
    {
        if (!candidate.namesTemporalBaseline) continue;
        if (candidate.optional || candidate.precedence != baseline.precedence || namedBaseline)
            throw std::logic_error("Conflicting temporal baseline identity: " + candidate.id);
        namedBaseline = candidate;
    }
    if (namedBaseline) baseline = *namedBaseline;
    std::erase_if(candidates, [](const auto& candidate) { return candidate.namesTemporalBaseline; });
    candidates.push_back(baseline);

    // In Lent obligatory memorials may only be commemorated (UNLYC 14, table 12).
    for (auto& candidate : candidates)
        if (baseline.precedence == PrecedenceLevel::PrivilegedWeekday && isLent(date) &&
            candidate.rank == CelebrationRank::Memorial)
        {
            candidate.rank = CelebrationRank::OptionalMemorial;
            candidate.precedence = PrecedenceLevel::OptionalMemorial;
            candidate.optional = true;
        }

    std::ranges::sort(candidates, [](const auto& left, const auto& right) {
        if (left.precedence != right.precedence) return left.precedence < right.precedence;
        return left.id < right.id;
    });
    const ObservanceCandidate* selected = nullptr;
    for (const auto& candidate : candidates)
    {
        if (candidate.optional) continue;
        if (!selected) selected = &candidate;
        else if (candidate.precedence == selected->precedence)
            throw std::logic_error("Equal-precedence celebrations: " + selected->id + " / " + candidate.id);
        else break;
    }
    LiturgicalDay result;
    result.date = date;
    result.season = getLiturgicalPeriod(date, calendar);
    result.observanceName = selected->name;
    result.rank = selected->rank;
    result.precedence = selected->precedence;
    result.observanceId = selected->id;
    result.colour = selected->colour;
    result.transferredFrom = selected->transferredFrom;
    if (result.precedence >= PrecedenceLevel::PrivilegedWeekday)
        for (const auto& candidate : candidates)
            if (candidate.optional) result.alternatives.push_back(candidate);
    return result;
}
} // namespace

LiturgicalDay resolveLiturgicalDay(std::chrono::year_month_day date,
    const std::vector<FixedCelebration>& celebrations, std::string_view calendarName)
{
    if (!date.ok() || int(date.year()) < 1583 || int(date.year()) > 4099)
        throw std::invalid_argument("Date must be valid and in the Gregorian range 1583–4099");
    if (calendarName != "grc" && calendarName != "europe" &&
        calendarName != "england" && calendarName != "wales")
        throw std::invalid_argument("Unknown liturgical calendar");
    validateCalendar(celebrations);
    auto candidates = dayCandidates(date, celebrations, calendarName);
    // Existing Advent transfer, replaced by the general schedule in phase C.
    const std::chrono::sys_days day{date};
    if (isAdvent(date) && std::chrono::weekday{day} == std::chrono::Monday)
        for (const auto& candidate : dayCandidates(std::chrono::year_month_day{day - std::chrono::days{1}}, celebrations, calendarName))
            if (candidate.rank == CelebrationRank::Solemnity)
                candidates.push_back(candidate);
    return selectDay(date, std::move(candidates), calendarName);
}
