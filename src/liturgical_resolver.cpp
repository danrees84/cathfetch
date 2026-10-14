#include "liturgical_resolver.h"
#include "calendar_loader.h"
#include "liturgical_format.h"
#include "movable_observances.h"
#include "easter.h"

#include <algorithm>
#include <stdexcept>
#include <set>

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
    if (celebration.dateRule == FixedDateRule::AllSouls &&
        (calendar == "england" || calendar == "wales") && weekday{sys_days{date}} == Sunday)
        return year_month_day{sys_days{date} + days{1}};
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

struct ScheduledCelebration
{
    const FixedCelebration* definition;
    year_month_day scheduled;
    year_month_day observed;
    std::string unresolvedReason;
};

// Construct a schedule for this call: no global cache that could mix calendar overlays
// or retain stale user-supplied definitions. Higher precedence is scheduled first.
std::vector<ScheduledCelebration> fixedSchedule(int year,
    const std::vector<FixedCelebration>& celebrations, std::string_view calendar)
{
    std::vector<ScheduledCelebration> schedule;
    for (const auto& celebration : celebrations)
    {
        const auto date = scheduledDate(celebration, year, calendar);
        if (date.ok()) schedule.push_back({&celebration, date, date, {}});
    }
    std::ranges::sort(schedule, [](const auto& left, const auto& right) {
        const auto lp = fixedCelebrationPrecedence(*left.definition);
        const auto rp = fixedCelebrationPrecedence(*right.definition);
        if (lp != rp) return lp < rp;
        if (left.scheduled != right.scheduled) return left.scheduled < right.scheduled;
        return left.definition->id < right.definition->id;
    });
    for (std::size_t i = 1; i < schedule.size(); ++i)
    {
        const auto& previous = schedule[i - 1];
        const auto& current = schedule[i];
        if (previous.definition->rank == CelebrationRank::Solemnity &&
            current.definition->rank == CelebrationRank::Solemnity &&
            previous.scheduled == current.scheduled &&
            fixedCelebrationPrecedence(*previous.definition) == fixedCelebrationPrecedence(*current.definition))
            throw std::logic_error("Coinciding fixed solemnities require a disposition: " +
                                   previous.definition->id + " / " + current.definition->id);
    }
    const sys_days easter{calculateEaster(year)};
    const auto blockingPrecedence = [&](year_month_day date, const ScheduledCelebration* self) {
        auto best = temporalBaselinePrecedence(date, calendar);
        for (const auto& movable : findMovableObservances(date, calendar))
            if (!movable.optional && movable.precedence < best) best = movable.precedence;
        for (const auto& fixed : schedule)
            if (&fixed != self && fixed.observed == date)
            {
                const auto level = fixedCelebrationPrecedence(*fixed.definition);
                if (level < best) best = level;
            }
        return best;
    };
    for (auto& fixed : schedule)
    {
        const auto& definition = *fixed.definition;
        if (definition.rank != CelebrationRank::Solemnity || definition.namesTemporalBaseline) continue;
        const auto precedence = fixedCelebrationPrecedence(definition);
        const auto blocker = blockingPrecedence(fixed.scheduled, &fixed);
        const sys_days original{fixed.scheduled};
        const auto available = [&](sys_days day) {
            return blockingPrecedence(year_month_day{day}, &fixed) > PrecedenceLevel::ProperFeast;
        };
        std::optional<sys_days> destination;
        // The Sacred Heart / Baptist occurrence has an explicit CDW disposition (2022).
        if (definition.transferRule == SolemnityTransferRule::BeforeSacredHeart &&
            original == easter + days{68})
            destination = original - days{1};
        else if (blocker > precedence) continue;
        else if (blocker == precedence)
        {
            fixed.unresolvedReason = "Equal-precedence solemnities require an authoritative disposition";
            continue;
        }
        else if (definition.transferRule == SolemnityTransferRule::SaintJoseph &&
                 original >= easter - days{7} && original < easter)
            destination = easter - days{8};
        else if ((definition.transferRule == SolemnityTransferRule::Annunciation ||
                  definition.transferRule == SolemnityTransferRule::AfterEasterOctave) &&
                 original >= easter - days{7} && original <= easter + days{7})
            destination = easter + days{8};
        else if (weekday{original} == Sunday &&
                 (isLent(fixed.scheduled) || isAdvent(fixed.scheduled) || isEaster(fixed.scheduled)))
            destination = original + days{1}; // UNLYC 5: following Monday.
        if (destination)
        {
            // An already transferred solemnity can occupy the first free Monday.
            while (!available(*destination)) *destination += days{1};
        }
        else
        {
            // UNLYC 60: nearest day outside classes 1–8; prefer following in a tie.
            for (int distance = 1; distance <= 370 && !destination; ++distance)
            {
                if (available(original + days{distance})) destination = original + days{distance};
                else if (available(original - days{distance})) destination = original - days{distance};
            }
        }
        if (!destination) throw std::logic_error("No transfer destination: " + definition.id);
        fixed.observed = year_month_day{*destination};
    }
    return schedule;
}

std::vector<ObservanceCandidate> dayCandidates(year_month_day date,
    const std::vector<FixedCelebration>& celebrations, std::string_view calendar)
{
    auto candidates = findMovableObservances(date, calendar);
    for (const auto& fixed : fixedSchedule(int(date.year()), celebrations, calendar))
    {
        if (fixed.observed != date) continue;
        auto candidate = fixedCandidate(*fixed.definition);
        candidate.unresolvedReason = fixed.unresolvedReason;
        if (fixed.observed != fixed.scheduled) candidate.transferredFrom = fixed.scheduled;
        const year_month_day nominal{date.year() / fixed.definition->date};
        if (fixed.scheduled != nominal) candidate.scheduledFrom = nominal;
        candidates.push_back(candidate);
    }
    return candidates;
}

LiturgicalDay selectDay(year_month_day date, std::vector<ObservanceCandidate> candidates,
                       std::string_view calendar)
{
    auto baseline = temporalCandidate(date, calendar);
    std::set<std::string> identities;
    for (const auto& candidate : candidates)
        if (candidate.id == "temporal" || !identities.insert(candidate.id).second)
            throw std::invalid_argument("Duplicate or reserved candidate identity: " + candidate.id);
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

    // CDW notification 2671/98/L: both memorials become optional on occurrence
    // with the Immaculate Heart. This does not demote unrelated memorial ties.
    const bool heartOccurrence = std::ranges::any_of(candidates, [](const auto& candidate) {
        return candidate.reducesMemorialCollision;
    }) && std::ranges::count_if(candidates, [](const auto& candidate) {
        return candidate.rank == CelebrationRank::Memorial;
    }) > 1;
    if (heartOccurrence)
        for (auto& candidate : candidates)
            if (candidate.rank == CelebrationRank::Memorial)
            {
                candidate.rank = CelebrationRank::OptionalMemorial;
                candidate.precedence = PrecedenceLevel::OptionalMemorial;
                candidate.optional = true;
            }

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
        if (left.prevailsOnEqualPrecedence != right.prevailsOnEqualPrecedence)
            return left.prevailsOnEqualPrecedence;
        return left.id < right.id;
    });
    const ObservanceCandidate* selected = nullptr;
    for (const auto& candidate : candidates)
    {
        if (candidate.optional) continue;
        if (!selected) selected = &candidate;
        else if (candidate.precedence == selected->precedence)
        {
            // Explicit dispositions: Mother of the Church (CDW 138/18) and
            // Eternal High Priest / Augustine (England and Wales Calendar Notes 2021).
            if (selected->prevailsOnEqualPrecedence && !candidate.prevailsOnEqualPrecedence &&
                candidate.rank == selected->rank) continue;
            if (!selected->unresolvedReason.empty() || !candidate.unresolvedReason.empty())
            {
                LiturgicalDay unresolved;
                unresolved.date = date;
                unresolved.season = getLiturgicalPeriod(date, calendar);
                unresolved.precedence = selected->precedence;
                unresolved.unresolvedReason = "Equal-precedence solemnities require an authoritative disposition";
                unresolved.observanceName = "Unresolved liturgical occurrence";
                for (const auto& rival : candidates)
                    if (!rival.optional && rival.precedence == selected->precedence)
                        unresolved.conflictingObservances.push_back(rival);
                return unresolved;
            }
            throw std::logic_error("Equal-precedence celebrations: " + selected->id + " / " + candidate.id);
        }
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
    result.scheduledFrom = selected->scheduledFrom;
    if (result.precedence == PrecedenceLevel::PrivilegedWeekday ||
        result.precedence == PrecedenceLevel::OtherWeekday)
        for (const auto& candidate : candidates)
            if (candidate.optional)
            {
                auto alternative = candidate;
                alternative.restrictedCommemoration = result.precedence == PrecedenceLevel::PrivilegedWeekday;
                result.alternatives.push_back(std::move(alternative));
            }
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
    return selectDay(date, std::move(candidates), calendarName);
}
