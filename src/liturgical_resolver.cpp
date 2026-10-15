#include "liturgical_resolver.h"
#include "calendar_loader.h"
#include "liturgical_format.h"
#include "movable_observances.h"
#include "easter.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
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
    candidate.subject = celebration.subject;
    candidate.principalPatron = celebration.principalPatron;
    return candidate;
}

// The 2022 CDW patronal exception reverses the anticipation. It is not a
// general permission to move Corpus Christi in an unspecified local calendar.
std::vector<ObservanceCandidate> scheduledMovables(year_month_day date,
    const std::vector<FixedCelebration>& celebrations, std::string_view calendar)
{
    auto candidates = findMovableObservances(date, calendar);
    const sys_days sacredHeart{sys_days{calculatePentecost(int(date.year()))} + days{19}};
    const bool patronalOccurrence = std::ranges::any_of(celebrations, [&](const auto& definition) {
        return definition.principalPatron &&
               definition.transferRule == SolemnityTransferRule::AnticipateBeforeLord &&
               scheduledDate(definition, int(date.year()), calendar) == year_month_day{sacredHeart};
    });
    if (!patronalOccurrence) return candidates;
    if (sys_days{date} == sacredHeart)
        std::erase_if(candidates, [](const auto& c) { return c.id == "sacred_heart"; });
    else if (sys_days{date} == sacredHeart - days{1})
        for (auto candidate : findMovableObservances(year_month_day{sacredHeart}, calendar))
            if (candidate.id == "sacred_heart")
            {
                candidate.transferredFrom = year_month_day{sacredHeart};
                candidate.transferNote = "Patronal exception to the 2022 CDW anticipation disposition";
                candidates.push_back(std::move(candidate));
            }
    return candidates;
}

struct ScheduledCelebration
{
    // Borrow the definition from the caller's vector; this schedule owns dates,
    // not another copy of each celebration. The input vector stays alive while
    // the schedule is being used.
    const FixedCelebration* definition;
    year_month_day scheduled;
    year_month_day observed;
    std::string unresolvedReason;
    bool provisionalTransfer = false;
    std::string transferNote = {};
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
        const auto leftPrecedence = fixedCelebrationPrecedence(*left.definition);
        const auto rightPrecedence = fixedCelebrationPrecedence(*right.definition);
        if (leftPrecedence != rightPrecedence) return leftPrecedence < rightPrecedence;
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
            fixedCelebrationPrecedence(*previous.definition) == fixedCelebrationPrecedence(*current.definition) &&
            !observanceOutranks(fixedCandidate(*previous.definition), fixedCandidate(*current.definition)) &&
            !observanceOutranks(fixedCandidate(*current.definition), fixedCandidate(*previous.definition)))
            throw std::logic_error("Coinciding fixed solemnities require a disposition: " +
                                   previous.definition->id + " / " + current.definition->id);
    }
    const sys_days easter{calculateEaster(year)};
    // Find the strongest competing day. Excluding "self" prevents a solemnity
    // from blocking its own date when we check possible transfer destinations.
    const auto blockingPrecedence = [&](year_month_day date, const ScheduledCelebration* self) {
        auto best = temporalBaselinePrecedence(date, calendar);
        for (const auto& movable : scheduledMovables(date, celebrations, calendar))
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
            // A transferred solemnity needs a day outside precedence classes 1–8.
            return blockingPrecedence(year_month_day{day}, &fixed) > PrecedenceLevel::ProperFeast;
        };
        std::optional<sys_days> destination;
        if (blocker > precedence) continue;
        if (definition.principalPatron && definition.transferRule == SolemnityTransferRule::AnticipateBeforeLord &&
            blocker >= PrecedenceLevel::GeneralSolemnityOrAllSouls &&
            std::ranges::any_of(scheduledMovables(fixed.scheduled, celebrations, calendar), [](const auto& rival) {
                return rival.subject == CelebrationSubject::Lord && rival.rank == CelebrationRank::Solemnity &&
                       rival.precedence == PrecedenceLevel::GeneralSolemnityOrAllSouls;
            }))
        {
            fixed.unresolvedReason = "Local patronal occurrence requires its own authoritative disposition";
            continue;
        }
        if (blocker == precedence)
        {
            const auto self = fixedCandidate(definition);
            std::vector<ObservanceCandidate> rivals = scheduledMovables(fixed.scheduled, celebrations, calendar);
            for (const auto& other : schedule)
                if (&other != &fixed && other.observed == fixed.scheduled)
                    rivals.push_back(fixedCandidate(*other.definition));
            ObservanceCandidate temporal;
            temporal.precedence = temporalBaselinePrecedence(fixed.scheduled, calendar);
            rivals.push_back(temporal);
            bool impeded = false;
            bool ambiguous = false;
            for (const auto& rival : rivals)
                if (!rival.optional && rival.precedence == precedence)
                {
                    const bool rivalWins = observanceOutranks(rival, self);
                    const bool selfWins = observanceOutranks(self, rival);
                    if (rivalWins) impeded = true;
                    if (!rivalWins && !selfWins) ambiguous = true;
                }
            if (!impeded)
            {
                if (ambiguous) fixed.unresolvedReason = "Equal-precedence solemnities require an authoritative disposition";
                continue;
            }
            if (definition.transferRule == SolemnityTransferRule::AnticipateBeforeLord)
            {
                destination = original - days{1};
                fixed.provisionalTransfer = original != easter + days{68};
                if (fixed.provisionalTransfer)
                    fixed.transferNote = "Provisional anticipation by analogy with the 2022 CDW disposition; transfer date awaits authoritative confirmation";
            }
        }
        if (!destination)
        {
            if (definition.transferRule == SolemnityTransferRule::SaintJoseph &&
                     original >= easter - days{7} && original < easter)
                destination = easter - days{8};
            else if ((definition.transferRule == SolemnityTransferRule::Annunciation ||
                      definition.transferRule == SolemnityTransferRule::AfterEasterOctave) &&
                     original >= easter - days{7} && original <= easter + days{7})
                destination = easter + days{8};
            else if (weekday{original} == Sunday &&
                     (isLent(fixed.scheduled) || isAdvent(fixed.scheduled) || isEaster(fixed.scheduled)))
                destination = original + days{1}; // UNLYC 5: following Monday.
        }
        if (destination)
        {
            // An already transferred solemnity can occupy the first free Monday.
            while (!available(*destination))
                *destination += days{*destination < original ? -1 : 1};
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
    const std::vector<FixedCelebration>& celebrations, std::string_view calendar,
    std::vector<ObservanceCandidate>& displaced)
{
    auto candidates = scheduledMovables(date, celebrations, calendar);
    for (const auto& fixed : fixedSchedule(int(date.year()), celebrations, calendar))
    {
        if (fixed.observed != date && fixed.scheduled != date) continue;
        auto candidate = fixedCandidate(*fixed.definition);
        candidate.provisionalTransfer = fixed.provisionalTransfer;
        candidate.transferNote = fixed.transferNote;
        if (fixed.observed != date)
        {
            candidate.transferredFrom = fixed.scheduled;
            candidate.transferredTo = fixed.observed;
            displaced.push_back(std::move(candidate));
            continue;
        }
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
    // A named baseline replaces the unnamed seasonal day. It is not an extra
    // competitor with the same precedence, which would create an artificial tie.
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

    // ID order stabilises diagnostics and alternatives, never liturgical priority.
    std::ranges::sort(candidates, {}, &ObservanceCandidate::id);
    std::vector<const ObservanceCandidate*> winners;
    for (const auto& candidate : candidates)
    {
        if (candidate.optional) continue;
        const bool beatenByRival = std::ranges::any_of(candidates, [&](const auto& rival) {
            return !rival.optional && observanceOutranks(rival, candidate);
        });
        if (!beatenByRival)
        {
            winners.push_back(&candidate);
        }
    }
    // A principal-patron exception needs jurisdiction-specific authority even
    // when the patron's proper class differs numerically from the general class.
    const bool localDisposition = std::ranges::any_of(candidates, [](const auto& candidate) {
        return candidate.principalPatron && !candidate.unresolvedReason.empty();
    });
    if (localDisposition)
    {
        winners.clear();
        for (const auto& candidate : candidates)
            if (candidate.rank == CelebrationRank::Solemnity &&
                candidate.precedence <= PrecedenceLevel::ProperSolemnity) winners.push_back(&candidate);
    }
    if (winners.size() != 1)
    {
        const bool requiresDisposition = std::ranges::any_of(winners, [](const auto* candidate) {
            return !candidate->unresolvedReason.empty();
        });
        if (!requiresDisposition) throw std::logic_error("Equal-precedence celebrations require an explicit disposition");
        LiturgicalDay unresolved;
        unresolved.date = date;
        unresolved.season = getLiturgicalPeriod(date, calendar);
        unresolved.precedence = winners.front()->precedence;
        unresolved.unresolvedReason = localDisposition
            ? "Local patronal occurrence requires its own authoritative disposition"
            : "Equal-precedence solemnities require an authoritative disposition";
        unresolved.observanceName = "Unresolved liturgical occurrence";
        for (const auto* winner : winners) unresolved.conflictingObservances.push_back(*winner);
        return unresolved;
    }
    const auto* selected = winners.front();
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
    result.provisionalTransfer = selected->provisionalTransfer;
    result.transferNote = selected->transferNote;
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
    std::vector<ObservanceCandidate> displaced;
    auto candidates = dayCandidates(date, celebrations, calendarName, displaced);
    auto result = selectDay(date, std::move(candidates), calendarName);
    result.displacedObservances = std::move(displaced);
    return result;
}
