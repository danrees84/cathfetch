// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include <iostream>
#include "liturgical_precedence.h"
#include "calendar_selection.h"
#include "calendar_lookup.h"
#include "temporal_calendar.h"

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

if (davidLewisPrecedence != PrecedenceLevel::OptionalMemorial ||
    !outranks(davidLewisPrecedence, augustWeekday))
{
    std::cerr << "Welsh optional memorial precedence test failed\n";
    return 1;
}

auto firstAdventSunday = temporalBaselinePrecedence(
    std::chrono::year{2026} / std::chrono::November / 29);

auto adventWeekday = temporalBaselinePrecedence(
    std::chrono::year{2026} / std::chrono::November / 30);

auto privilegedWeekday = temporalBaselinePrecedence(
    std::chrono::year{2026} / std::chrono::December / 17);

auto fourthAdventSunday = temporalBaselinePrecedence(
    std::chrono::year{2026} / std::chrono::December / 20);

if (firstAdventSunday != PrecedenceLevel::HighestDays ||
    adventWeekday != PrecedenceLevel::OtherWeekday ||
    privilegedWeekday != PrecedenceLevel::PrivilegedWeekday ||
    fourthAdventSunday != PrecedenceLevel::HighestDays)
{
    std::cerr << "Advent precedence test failed\n";
    return 1;
}

if (temporalBaselinePrecedence(
        std::chrono::year{2026} / std::chrono::February / 18)
        != PrecedenceLevel::HighestDays ||
    temporalBaselinePrecedence(
        std::chrono::year{2026} / std::chrono::February / 22)
        != PrecedenceLevel::HighestDays ||
    temporalBaselinePrecedence(
        std::chrono::year{2026} / std::chrono::February / 23)
        != PrecedenceLevel::PrivilegedWeekday ||
    temporalBaselinePrecedence(
        std::chrono::year{2026} / std::chrono::March / 30)
        != PrecedenceLevel::HighestDays)
{
    std::cerr << "Lent precedence test failed\n";
    return 1;
}

if (temporalBaselinePrecedence(
        std::chrono::year{2026} / std::chrono::April / 2)
        != PrecedenceLevel::HighestDays ||
    temporalBaselinePrecedence(
        std::chrono::year{2026} / std::chrono::April / 3)
        != PrecedenceLevel::Triduum ||
    temporalBaselinePrecedence(
        std::chrono::year{2026} / std::chrono::April / 4)
        != PrecedenceLevel::Triduum ||
    temporalBaselinePrecedence(
        std::chrono::year{2026} / std::chrono::April / 5)
        != PrecedenceLevel::Triduum)
{
    std::cerr << "Easter Triduum precedence test failed\n";
    return 1;
}


struct EasterTestCase
{
    std::chrono::year_month_day date;
    PrecedenceLevel expected;
};

const EasterTestCase easterTests[] = {
    {std::chrono::year{2026} / std::chrono::April / 5,
     PrecedenceLevel::Triduum},

    {std::chrono::year{2026} / std::chrono::April / 6,
     PrecedenceLevel::HighestDays},

    {std::chrono::year{2026} / std::chrono::April / 11,
     PrecedenceLevel::HighestDays},

    {std::chrono::year{2026} / std::chrono::April / 12,
     PrecedenceLevel::HighestDays},

    {std::chrono::year{2026} / std::chrono::April / 13,
     PrecedenceLevel::OtherWeekday}
};

for (const auto& test : easterTests)
{
    if (temporalBaselinePrecedence(test.date) != test.expected)
    {
        std::cerr << "Easter precedence test failed: "
                  << test.date << "\n";
        return 1;
    }
}


struct AscensionTestCase
{
    int year;
    std::chrono::year_month_day expectedDate;
};

const AscensionTestCase ascensionTests[] = {
    {2025, std::chrono::year{2025} / std::chrono::May / 29},
    {2026, std::chrono::year{2026} / std::chrono::May / 14},
    {2027, std::chrono::year{2027} / std::chrono::May / 6}
};

for (const auto& test : ascensionTests)
{
    auto ascensionDate = calculateAscension(test.year);

    if (ascensionDate != test.expectedDate ||
        temporalBaselinePrecedence(ascensionDate) !=
            PrecedenceLevel::HighestDays)
    {
        std::cerr << "Ascension test failed for "
                  << test.year << "\n";
        return 1;
    }
}

auto beforeAscension = temporalBaselinePrecedence(
    std::chrono::year{2026} / std::chrono::May / 13);

if (beforeAscension != PrecedenceLevel::OtherWeekday)
{
    std::cerr << "Pre-Ascension weekday test failed\n";
    return 1;
}


auto pentecost2026 = calculatePentecost(2026);

if (pentecost2026 !=
        (std::chrono::year{2026} / std::chrono::May / 24) ||
    temporalBaselinePrecedence(pentecost2026) !=
        PrecedenceLevel::HighestDays)
{
    std::cerr << "Pentecost precedence test failed\n";
    return 1;
}

auto mondayAfterPentecost = temporalBaselinePrecedence(
    std::chrono::year{2026} / std::chrono::May / 25);

if (mondayAfterPentecost != PrecedenceLevel::OtherWeekday)
{
    std::cerr << "Post-Pentecost precedence test failed\n";
    return 1;
}


struct ChristmasTestCase
{
    std::chrono::year_month_day date;
    PrecedenceLevel expected;
};

const ChristmasTestCase christmasTests[] = {
    {std::chrono::year{2026} / std::chrono::December / 25,
     PrecedenceLevel::HighestDays},

    {std::chrono::year{2026} / std::chrono::December / 26,
     PrecedenceLevel::PrivilegedWeekday},

    {std::chrono::year{2026} / std::chrono::December / 27,
     PrecedenceLevel::SundayOfChristmasOrOrdinaryTime},

    {std::chrono::year{2027} / std::chrono::January / 1,
     PrecedenceLevel::PrivilegedWeekday},

    {std::chrono::year{2027} / std::chrono::January / 2,
     PrecedenceLevel::OtherWeekday},

    {std::chrono::year{2027} / std::chrono::January / 3,
     PrecedenceLevel::SundayOfChristmasOrOrdinaryTime}
};

for (const auto& test : christmasTests)
{
    if (temporalBaselinePrecedence(test.date) != test.expected)
    {
        std::cerr << "Christmas precedence test failed: "
                  << test.date << "\n";
        return 1;
    }
}

struct HolyFamilyTestCase
{
    int year;
    std::chrono::year_month_day expectedDate;
};

const HolyFamilyTestCase holyFamilyTests[] = {
    {2022, std::chrono::year{2022} / std::chrono::December / 30},
    {2025, std::chrono::year{2025} / std::chrono::December / 28},
    {2026, std::chrono::year{2026} / std::chrono::December / 27}
};

for (const auto& test : holyFamilyTests)
{
    auto actualDate = calculateHolyFamily(test.year);

    if (actualDate != test.expectedDate)
    {
        std::cerr << "Holy Family date test failed for "
                  << test.year << "\n";
        return 1;
    }
}

struct EpiphanyTestCase
{
    int year;
    std::string_view calendar;
    std::chrono::year_month_day expectedDate;
};

const EpiphanyTestCase epiphanyTests[] = {
    {2024, "grc",
     std::chrono::year{2024} / std::chrono::January / 6},

    {2024, "england",
     std::chrono::year{2024} / std::chrono::January / 7},

    {2025, "grc",
     std::chrono::year{2025} / std::chrono::January / 6},

    {2025, "england",
     std::chrono::year{2025} / std::chrono::January / 5},

    {2026, "wales",
     std::chrono::year{2026} / std::chrono::January / 6},

    {2026, "europe",
     std::chrono::year{2026} / std::chrono::January / 6}
};

for (const auto& test : epiphanyTests)
{
    auto actualDate = calculateEpiphany(
        test.year, test.calendar);

    if (actualDate != test.expectedDate)
    {
        std::cerr << "Epiphany test failed for "
                  << test.year << " ("
                  << test.calendar << ")\n";
        return 1;
    }
}

struct BaptismTestCase
{
    int year;
    std::string_view calendar;
    std::chrono::year_month_day expectedDate;
};

const BaptismTestCase baptismTests[] = {
    {2024, "grc",
     std::chrono::year{2024} / std::chrono::January / 7},

    {2024, "england",
     std::chrono::year{2024} / std::chrono::January / 8},

    {2025, "england",
     std::chrono::year{2025} / std::chrono::January / 12},

    {2026, "wales",
     std::chrono::year{2026} / std::chrono::January / 11},

    {2030, "grc",
     std::chrono::year{2030} / std::chrono::January / 13}
};

for (const auto& test : baptismTests)
{
    auto actualDate = calculateBaptismOfLord(
        test.year, test.calendar);

    if (actualDate != test.expectedDate)
    {
        std::cerr << "Baptism of the Lord test failed for "
                  << test.year << " ("
                  << test.calendar << ")\n";

        return 1;
    }
}

auto january8 = std::chrono::year{2024} /
    std::chrono::January / 8;

if (isChristmas(january8, "grc") ||
    !isChristmas(january8, "england") ||
    !isChristmas(january8, "wales"))
{
    std::cerr << "Calendar-specific Christmas test failed\n";
    return 1;
}

auto january8_2024 = std::chrono::year{2024} /
    std::chrono::January / 8;

if (!isOrdinaryTime(january8_2024, "grc") ||
    isOrdinaryTime(january8_2024, "england") ||
    isOrdinaryTime(january8_2024, "wales"))
{
    std::cerr << "Calendar-specific Ordinary Time test failed\n";
    return 1;
}

auto testDate = std::chrono::year{2024} /
    std::chrono::January / 8;

if (getLiturgicalPeriod(testDate, "grc") !=
        LiturgicalPeriod::OrdinaryTime ||
    getLiturgicalPeriod(testDate, "england") !=
        LiturgicalPeriod::Christmas ||
    getLiturgicalPeriod(testDate, "wales") !=
        LiturgicalPeriod::Christmas)
{
    std::cerr << "Calendar-specific liturgical period test failed\n";
    return 1;
}

struct OrdinaryTimeWeekTest
{
    std::chrono::year_month_day date;
    std::string_view calendar;
    int expectedWeek;
};

const OrdinaryTimeWeekTest ordinaryTimeWeekTests[] = {
    {std::chrono::year{2024} / std::chrono::January / 8,
     "grc", 1},

    {std::chrono::year{2024} / std::chrono::January / 9,
     "england", 1},

    {std::chrono::year{2024} / std::chrono::January / 14,
     "england", 2},

    {std::chrono::year{2024} / std::chrono::January / 15,
     "wales", 2},

    {std::chrono::year{2025} / std::chrono::January / 13,
     "england", 1},

    {std::chrono::year{2025} / std::chrono::January / 19,
     "england", 2}
};

for (const auto& test : ordinaryTimeWeekTests)
{
    int actualWeek = calculateOrdinaryTimeWeek(
        test.date, test.calendar);

    if (actualWeek != test.expectedWeek)
    {
        std::cerr << "Ordinary Time week test failed: "
                  << test.date << " ("
                  << test.calendar << ")\n";
        return 1;
    }
}

std::cout << "All precedence tests passed\n";
    return 0;
}
