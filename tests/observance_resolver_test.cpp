#include <iostream>
#include <string>
#include "calendar_selection.h"
#include "liturgical_resolver.h"
#include <algorithm>
#include "calendar_lookup.h"
#include <stdexcept>

int main()
{
    auto celebrations = loadSelectedCalendar("grc");



    auto lawrenceDay = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::August / 10,
    celebrations);

if (lawrenceDay.observanceName !=
        "19th Sunday in Ordinary Time" ||
    lawrenceDay.precedence !=
        PrecedenceLevel::SundayOfChristmasOrOrdinaryTime)
{
    std::cerr << "St Lawrence observance test failed\n";
    return 1;
}




auto welshCalendar = loadSelectedCalendar("wales");









auto reversedWelshCalendar = welshCalendar;

std::ranges::reverse(reversedWelshCalendar);

auto forwardJuly23 = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::July / 23,
    welshCalendar);

auto reversedJuly23 = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::July / 23,
    reversedWelshCalendar);

if (reversedJuly23.observanceName != forwardJuly23.observanceName ||
    reversedJuly23.precedence != forwardJuly23.precedence)
{
    std::cerr << "Candidate order independence test failed\n";
    return 1;
}


auto leoMatches = findFixedCelebrations(
    celebrations,
    std::chrono::November / std::chrono::day{10});

if (leoMatches.size() != 1)
{
    std::cerr << "St Leo lookup failed\n";
    return 1;
}

auto tiedCandidates = leoMatches;

auto secondMemorial = leoMatches.front();
secondMemorial.id = "test-second-memorial";
secondMemorial.name = "Test Second Memorial";

tiedCandidates.push_back(secondMemorial);

bool tieRejected = false;

try
{
    resolveLiturgicalDay(
        std::chrono::year{2025} / std::chrono::November / 10,
        tiedCandidates);
}
catch (const std::logic_error&)
{
    tieRejected = true;
}

if (!tieRejected)
{
    std::cerr << "Equal-precedence test failed\n";
    return 1;
}


auto lateranDay = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::November / 9,
    celebrations);

auto ordinarySunday = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::October / 11,
    celebrations);

if (lateranDay.observanceName !=
        "The Dedication of the Lateran Basilica" ||
    ordinarySunday.observanceName !=
        "28th Sunday in Ordinary Time")
{
    std::cerr << "Liturgical day resolution test failed\n";
    return 1;
}

auto stDavidDay = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::March / 1,
    welshCalendar);

if (stDavidDay.observanceName !=
        "St David, Bishop, Patron of Wales" ||
    stDavidDay.precedence != PrecedenceLevel::ProperSolemnity)
{
    std::cerr << "Welsh liturgical day test failed\n";
    return 1;
}

auto stLeoDay = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::November / 10,
    celebrations);

if (stLeoDay.observanceName !=
        "St Leo the Great, Pope and Doctor of the Church" ||
    stLeoDay.precedence != PrecedenceLevel::GeneralMemorial)
{
    std::cerr << "General memorial liturgical day test failed\n";
    return 1;
}

auto welshJuly23 = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::July / 23,
    welshCalendar);

if (welshJuly23.observanceName != "St Bridget, Religious" ||
    welshJuly23.precedence != PrecedenceLevel::ProperFeast)
{
    std::cerr << "Welsh multiple-observance test failed\n";
    return 1;
}

std::cout << "Observance resolver tests passed\n";


    return 0;
}
