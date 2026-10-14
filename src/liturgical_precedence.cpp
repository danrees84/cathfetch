#include "liturgical_precedence.h"
#include "temporal_calendar.h"
#include "easter.h"

#include <stdexcept>

bool outranks(PrecedenceLevel candidate, PrecedenceLevel current)
{
    return candidate < current;
}

PrecedenceLevel ordinaryTimeBaselinePrecedence(
    std::chrono::year_month_day date, std::string_view calendarName)
{
    if (!date.ok() || !isOrdinaryTime(date, calendarName))
        throw std::invalid_argument("Date is not a valid Ordinary Time date");
    return std::chrono::weekday{std::chrono::sys_days{date}} == std::chrono::Sunday
        ? PrecedenceLevel::SundayOfChristmasOrOrdinaryTime : PrecedenceLevel::OtherWeekday;
}

PrecedenceLevel temporalBaselinePrecedence(
    std::chrono::year_month_day date, std::string_view calendarName)
{
    using namespace std::chrono;
    if (!date.ok()) throw std::invalid_argument("Invalid calendar date");
    const int year = int(date.year());
    const sys_days day{date};
    const bool sunday = weekday{day} == Sunday;
    if (isTriduum(date)) return PrecedenceLevel::Triduum;
    if (date == calculateEpiphany(year, calendarName) || date == calculateAscension(year))
        return PrecedenceLevel::HighestDays;

    switch (getLiturgicalPeriod(date, calendarName))
    {
        case LiturgicalPeriod::Advent:
            if (sunday) return PrecedenceLevel::HighestDays;
            return date.month() == December && unsigned(date.day()) >= 17
                ? PrecedenceLevel::PrivilegedWeekday : PrecedenceLevel::OtherWeekday;
        case LiturgicalPeriod::Christmas:
            if (date.month() == December && unsigned(date.day()) == 25)
                return PrecedenceLevel::HighestDays;
            if (sunday) return PrecedenceLevel::SundayOfChristmasOrOrdinaryTime;
            if (date.month() == December || unsigned(date.day()) == 1)
                return PrecedenceLevel::PrivilegedWeekday;
            return PrecedenceLevel::OtherWeekday;
        case LiturgicalPeriod::Lent:
            if (sunday || date == calculateAshWednesday(year) || date > calculatePalmSunday(year))
                return PrecedenceLevel::HighestDays;
            return PrecedenceLevel::PrivilegedWeekday;
        case LiturgicalPeriod::Easter:
            if (sunday || day <= sys_days{calculateEaster(year)} + days{7})
                return PrecedenceLevel::HighestDays;
            return PrecedenceLevel::OtherWeekday;
        case LiturgicalPeriod::OrdinaryTime:
            return sunday ? PrecedenceLevel::SundayOfChristmasOrOrdinaryTime : PrecedenceLevel::OtherWeekday;
        case LiturgicalPeriod::Triduum:
            return PrecedenceLevel::Triduum;
    }
    throw std::logic_error("Unknown liturgical season");
}

PrecedenceLevel fixedCelebrationPrecedence(const FixedCelebration& celebration)
{
    // Highest solemnities and All Souls have explicit classifications in the data.
    if (celebration.precedence)
    {
        const int level = int(*celebration.precedence);
        if (level < 1 || level > 13) throw std::invalid_argument("Invalid precedence: " + celebration.id);
        return *celebration.precedence;
    }
    if (!celebration.rank)
        throw std::invalid_argument("Celebration has no precedence classification: " + celebration.id);
    const bool general = celebration.rankSourceCalendar == "general_roman";
    switch (*celebration.rank)
    {
        case CelebrationRank::Solemnity:
            return general ? PrecedenceLevel::GeneralSolemnityOrAllSouls : PrecedenceLevel::ProperSolemnity;
        case CelebrationRank::Feast:
            if (!general) return PrecedenceLevel::ProperFeast;
            return celebration.feastOfLord ? PrecedenceLevel::GeneralFeastOfLord : PrecedenceLevel::GeneralFeast;
        case CelebrationRank::Memorial:
            return general ? PrecedenceLevel::GeneralMemorial : PrecedenceLevel::ProperMemorial;
        case CelebrationRank::OptionalMemorial:
            return PrecedenceLevel::OptionalMemorial;
    }
    throw std::invalid_argument("Unknown celebration rank: " + celebration.id);
}
