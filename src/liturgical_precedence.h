#ifndef LITURGICAL_PRECEDENCE_H
#define LITURGICAL_PRECEDENCE_H

#include <chrono>
#include "fixed_celebration.h"
#include <string_view>

// Lower numbers take priority. Rank and precedence are different: a solemnity
// does not always belong to the same numbered class.
enum class PrecedenceLevel : int
{
    Triduum = 1,
    HighestDays = 2,
    GeneralSolemnityOrAllSouls = 3,
    ProperSolemnity = 4,
    GeneralFeastOfLord = 5,
    SundayOfChristmasOrOrdinaryTime = 6,
    GeneralFeast = 7,
    ProperFeast = 8,
    PrivilegedWeekday = 9,
    GeneralMemorial = 10,
    ProperMemorial = 11,
    OptionalMemorial = 12,
    OtherWeekday = 13
};

struct ObservanceCandidate;
bool observanceOutranks(const ObservanceCandidate& candidate, const ObservanceCandidate& current);

bool outranks(PrecedenceLevel candidate, PrecedenceLevel current);

PrecedenceLevel ordinaryTimeBaselinePrecedence(
    std::chrono::year_month_day date,
    std::string_view calendarName = "grc");

PrecedenceLevel temporalBaselinePrecedence(
    std::chrono::year_month_day date,
    std::string_view calendarName = "grc");

PrecedenceLevel fixedCelebrationPrecedence(
    const FixedCelebration& celebration);

#endif
