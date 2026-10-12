#include <iostream>
#include "liturgical_precedence.h"
#include "calendar_selection.h"
#include "calendar_lookup.h"

int main()
{
    bool sundayWins = outranks(
        PrecedenceLevel::SundayOfChristmasOrOrdinaryTime,
        PrecedenceLevel::OptionalMemorial);

    bool memorialWins = outranks(
        PrecedenceLevel::OptionalMemorial,
        PrecedenceLevel::SundayOfChristmasOrOrdinaryTime);

    bool solemnityWins = outranks(
        PrecedenceLevel::ProperSolemnity,
        PrecedenceLevel::OtherWeekday);

    if (!sundayWins || memorialWins || !solemnityWins)
    {
        std::cerr << "Precedence test failed\n";
        return 1;
    }

auto sunday = ordinaryTimeBaselinePrecedence(
    std::chrono::year{2026} / std::chrono::October / 11);

auto weekday = ordinaryTimeBaselinePrecedence(
    std::chrono::year{2025} / std::chrono::March / 1);

if (sunday != PrecedenceLevel::SundayOfChristmasOrOrdinaryTime ||
    weekday != PrecedenceLevel::OtherWeekday)
{
    std::cerr << "Ordinary Time baseline test failed\n";
    return 1;
}

auto celebrations = loadSelectedCalendar("grc");

auto matches = findFixedCelebrations(
    celebrations,
    std::chrono::October / std::chrono::day{11});

if (matches.size() != 1 ||
    matches.front().name != "St John XXIII, Pope")
{
    std::cerr << "St John XXIII lookup failed\n";
    return 1;
}

auto sundayPrecedence = ordinaryTimeBaselinePrecedence(
    std::chrono::year{2026} / std::chrono::October / 11);

auto memorialPrecedence =
    fixedCelebrationPrecedence(matches.front());

if (!outranks(sundayPrecedence, memorialPrecedence))
{
    std::cerr << "Sunday precedence test failed\n";
    return 1;
}

auto saturdayPrecedence = ordinaryTimeBaselinePrecedence(
    std::chrono::year{2025} / std::chrono::October / 11);

if (!outranks(memorialPrecedence, saturdayPrecedence))
{
    std::cerr << "Optional memorial precedence test failed\n";
    return 1;
}


auto status2026 = optionalMemorialStatusInOrdinaryTime(
    std::chrono::year{2026} / std::chrono::October / 11);

auto status2025 = optionalMemorialStatusInOrdinaryTime(
    std::chrono::year{2025} / std::chrono::October / 11);

if (status2026 != ObservanceStatus::Impeded ||
    status2025 != ObservanceStatus::Optional)
{
    std::cerr << "Optional memorial status test failed\n";
    return 1;
}

auto assessment2026 = assessOptionalMemorialInOrdinaryTime(
    matches.front(),
    std::chrono::year{2026} / std::chrono::October / 11);

auto assessment2025 = assessOptionalMemorialInOrdinaryTime(
    matches.front(),
    std::chrono::year{2025} / std::chrono::October / 11);

if (assessment2026.status != ObservanceStatus::Impeded ||
    assessment2025.status != ObservanceStatus::Optional)
{
    std::cerr << "Celebration assessment test failed\n";
    return 1;
}


auto welshCalendar = loadSelectedCalendar("wales");

auto stDavidMatches = findFixedCelebrations(
    welshCalendar,
    std::chrono::March / std::chrono::day{1});

if (stDavidMatches.size() != 1)
{
    std::cerr << "St David lookup failed\n";
    return 1;
}

auto stDavidPrecedence =
    fixedCelebrationPrecedence(stDavidMatches.front());

auto weekdayPrecedence = ordinaryTimeBaselinePrecedence(
    std::chrono::year{2025} / std::chrono::March / 1);

if (stDavidPrecedence != PrecedenceLevel::ProperSolemnity ||
    !outranks(stDavidPrecedence, weekdayPrecedence))
{
    std::cerr << "St David precedence test failed\n";
    return 1;
}

auto transfiguration = findFixedCelebrations(
    celebrations,
    std::chrono::August / std::chrono::day{6});

auto stLawrence = findFixedCelebrations(
    celebrations,
    std::chrono::August / std::chrono::day{10});

if (transfiguration.size() != 1 || stLawrence.size() != 1)
{
    std::cerr << "Feast lookup failed\n";
    return 1;
}

auto transfigurationPrecedence =
    fixedCelebrationPrecedence(transfiguration.front());

auto stLawrencePrecedence =
    fixedCelebrationPrecedence(stLawrence.front());

if (transfigurationPrecedence != PrecedenceLevel::GeneralFeastOfLord ||
    stLawrencePrecedence != PrecedenceLevel::GeneralFeast)
{
    std::cerr << "Feast precedence test failed\n";
    return 1;
}

auto europeanCalendar = loadSelectedCalendar("europe");

auto stBenedict = findFixedCelebrations(
    europeanCalendar,
    std::chrono::July / std::chrono::day{11});

if (stBenedict.size() != 1 ||
    fixedCelebrationPrecedence(stBenedict.front()) !=
        PrecedenceLevel::ProperFeast)
{
    std::cerr << "European feast precedence test failed\n";
    return 1;
}


auto lateran = findFixedCelebrations(
    celebrations,
    std::chrono::November / std::chrono::day{9});

auto lawrence = findFixedCelebrations(
    celebrations,
    std::chrono::August / std::chrono::day{10});

if (lateran.size() != 1 || lawrence.size() != 1)
{
    std::cerr << "Sunday feast lookup failed\n";
    return 1;
}

auto novemberSunday = ordinaryTimeBaselinePrecedence(
    std::chrono::year{2025} / std::chrono::November / 9);

auto augustSunday = ordinaryTimeBaselinePrecedence(
    std::chrono::year{2025} / std::chrono::August / 10);

if (!outranks(fixedCelebrationPrecedence(lateran.front()),
              novemberSunday) ||
    !outranks(augustSunday,
              fixedCelebrationPrecedence(lawrence.front())))
{
    std::cerr << "Sunday feast precedence test failed\n";
    return 1;
}


auto davidLewis = findFixedCelebrations(
    welshCalendar,
    std::chrono::August / std::chrono::day{26});

if (davidLewis.size() != 1)
{
    std::cerr << "St David Lewis lookup failed\n";
    return 1;
}

auto davidLewisPrecedence =
    fixedCelebrationPrecedence(davidLewis.front());

auto augustWeekday = ordinaryTimeBaselinePrecedence(
    std::chrono::year{2026} / std::chrono::August / 26);

if (davidLewisPrecedence != PrecedenceLevel::ProperMemorial ||
    !outranks(davidLewisPrecedence, augustWeekday))
{
    std::cerr << "Proper memorial precedence test failed\n";
    return 1;
}

    std::cout << "All precedence tests passed\n";
    return 0;
}
