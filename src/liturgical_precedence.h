#ifndef LITURGICAL_PRECEDENCE_H
#define LITURGICAL_PRECEDENCE_H

#include <chrono>
#include "fixed_celebration.h"

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

enum class ObservanceStatus
{
    Observed,
    Optional,
    Impeded
};

bool outranks(PrecedenceLevel candidate, PrecedenceLevel current);
PrecedenceLevel ordinaryTimeBaselinePrecedence(
    std::chrono::year_month_day date);

PrecedenceLevel fixedCelebrationPrecedence(
    const FixedCelebration& celebration);

ObservanceStatus optionalMemorialStatusInOrdinaryTime(
    std::chrono::year_month_day date);

struct CelebrationAssessment
{
    FixedCelebration celebration;
    ObservanceStatus status;
};

CelebrationAssessment assessOptionalMemorialInOrdinaryTime(
    const FixedCelebration& celebration,
    std::chrono::year_month_day date);

#endif
