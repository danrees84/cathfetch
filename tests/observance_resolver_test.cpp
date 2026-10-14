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
			welshCalendar, "wales");

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


	auto immaculateConceptionDay = resolveLiturgicalDay(
			std::chrono::year{2026} / std::chrono::December / 8,
			celebrations);

	if (immaculateConceptionDay.precedence !=
			PrecedenceLevel::GeneralSolemnityOrAllSouls ||
			immaculateConceptionDay.observanceName.find(
				"Immaculate Conception") == std::string::npos)
	{
		std::cerr << "Immaculate Conception test failed\n";
		return 1;
	}


	auto adventSunday2024 = resolveLiturgicalDay(
			std::chrono::year{2024} / std::chrono::December / 8,
			celebrations);

	auto immaculateConception2024 = resolveLiturgicalDay(
			std::chrono::year{2024} / std::chrono::December / 9,
			celebrations);

	if (adventSunday2024.observanceName != "Second Sunday of Advent" ||
			immaculateConception2024.observanceName.find(
				"Immaculate Conception") == std::string::npos ||
			immaculateConception2024.precedence !=
			PrecedenceLevel::GeneralSolemnityOrAllSouls)
	{
		std::cerr << "Immaculate Conception transfer test failed\n";
		return 1;
	}

	auto december9_2026 = resolveLiturgicalDay(
			std::chrono::year{2026} / std::chrono::December / 9,
			celebrations);

	if (december9_2026.observanceName !=
			"Wednesday of the Second Week of Advent")
	{
		std::cerr << "Unnecessary solemnity transfer detected\n";
		return 1;
	}

auto holyFamilyDay = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::December / 27,
    celebrations);

if (holyFamilyDay.observanceName !=
        "The Holy Family of Jesus, Mary and Joseph" ||
    holyFamilyDay.precedence != PrecedenceLevel::GeneralFeastOfLord)
{
    std::cerr << "Holy Family resolution test failed\n";
    return 1;
}


auto romanEpiphany = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::January / 6,
    celebrations,
    "grc");

auto englishEpiphanyCalendar = loadSelectedCalendar("england");

auto englishEpiphany = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::January / 5,
    englishEpiphanyCalendar,
    "england");

if (romanEpiphany.observanceName.find("Epiphany") ==
        std::string::npos ||
    englishEpiphany.observanceName.find("Epiphany") ==
        std::string::npos ||
    romanEpiphany.precedence != PrecedenceLevel::HighestDays ||
    englishEpiphany.precedence != PrecedenceLevel::HighestDays)
{
    std::cerr << "Epiphany resolution test failed\n";
    return 1;
}


auto romanBaptism = resolveLiturgicalDay(
    std::chrono::year{2024} / std::chrono::January / 7,
    celebrations,
    "grc");

auto englishBaptism = resolveLiturgicalDay(
    std::chrono::year{2024} / std::chrono::January / 8,
    englishEpiphanyCalendar,
    "england");

if (romanBaptism.observanceName != "The Baptism of the Lord" ||
    englishBaptism.observanceName != "The Baptism of the Lord" ||
    romanBaptism.precedence != PrecedenceLevel::GeneralFeastOfLord ||
    englishBaptism.precedence != PrecedenceLevel::GeneralFeastOfLord)
{
    std::cerr << "Baptism resolution test failed\n";
    return 1;
}

auto christmasDay = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::December / 25,
    celebrations,
    "grc");

if (christmasDay.precedence != PrecedenceLevel::HighestDays ||
    christmasDay.observanceName.find("Nativity") == std::string::npos)
{
    std::cerr << "Christmas Day resolution test failed\n";
    return 1;
}

auto christmasWeekday = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::December / 30,
    celebrations,
    "grc");


if (christmasWeekday.observanceName !=
        "Sixth Day within the Octave of the Nativity of the Lord" ||
    christmasWeekday.precedence != PrecedenceLevel::PrivilegedWeekday)
{
    std::cerr << "Christmas weekday resolution test failed\n";
    return 1;
}

auto stStephenDay = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::December / 26,
    celebrations,
    "grc");

if (stStephenDay.observanceName !=
        "St Stephen, the First Martyr" ||
    stStephenDay.precedence != PrecedenceLevel::GeneralFeast)
{
    std::cerr << "St Stephen resolution test failed\n";
    return 1;
}

auto motherOfGodDay = resolveLiturgicalDay(
    std::chrono::year{2027} / std::chrono::January / 1,
    celebrations,
    "grc");

if (motherOfGodDay.rank != CelebrationRank::Solemnity ||
    motherOfGodDay.precedence !=
        PrecedenceLevel::GeneralSolemnityOrAllSouls)
{
    std::cerr << "Mary Mother of God resolution test failed\n";
    return 1;
}



auto secondChristmasSunday = resolveLiturgicalDay(
    std::chrono::year{2027} / std::chrono::January / 3,
    celebrations,
    "grc");

if (secondChristmasSunday.observanceName !=
        "Second Sunday after Christmas" ||
    secondChristmasSunday.precedence !=
        PrecedenceLevel::SundayOfChristmasOrOrdinaryTime)
{
    std::cerr << "Second Sunday after Christmas test failed\n";
    return 1;
}

auto englishJanuary6 = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::January / 6,
    englishEpiphanyCalendar,
    "england");

if (englishJanuary6.observanceName !=
        "Monday after Epiphany")
{
    std::cerr << "Post-Epiphany weekday test failed\n";
    return 1;
}

auto trinitySunday = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::May / 31,
    celebrations,
    "grc");

if (trinitySunday.observanceName != "The Most Holy Trinity" ||
    trinitySunday.rank != CelebrationRank::Solemnity ||
    trinitySunday.precedence !=
        PrecedenceLevel::GeneralSolemnityOrAllSouls)
{
    std::cerr << "Trinity Sunday resolution test failed\n";
    return 1;
}


auto trinitySunday2025 = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::June / 15,
    celebrations,
    "grc");

if (trinitySunday2025.observanceName != "The Most Holy Trinity" ||
    trinitySunday2025.rank != CelebrationRank::Solemnity ||
    trinitySunday2025.precedence !=
        PrecedenceLevel::GeneralSolemnityOrAllSouls)
{
    std::cerr << "Trinity Sunday 2025 test failed\n";
    return 1;
}
auto corpusChristiRoman = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::June / 4,
    celebrations,
    "grc");

if (corpusChristiRoman.observanceName !=
        "The Most Holy Body and Blood of Christ" ||
    corpusChristiRoman.rank != CelebrationRank::Solemnity ||
    corpusChristiRoman.precedence !=
        PrecedenceLevel::GeneralSolemnityOrAllSouls)
{
    std::cerr << "Corpus Christi GRC test failed\n";
    return 1;
}

auto corpusChristiEngland = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::June / 7,
    englishEpiphanyCalendar,
    "england");

if (corpusChristiEngland.observanceName !=
        "The Most Holy Body and Blood of Christ" ||
    corpusChristiEngland.rank != CelebrationRank::Solemnity ||
    corpusChristiEngland.precedence !=
        PrecedenceLevel::GeneralSolemnityOrAllSouls)
{
    std::cerr << "Corpus Christi England test failed\n";
    return 1;
}

auto corpusChristiNotRomanSunday = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::June / 7,
    celebrations,
    "grc");

if (corpusChristiNotRomanSunday.observanceName ==
        "The Most Holy Body and Blood of Christ")
{
    std::cerr << "Corpus Christi incorrectly moved in GRC\n";
    return 1;
}

auto corpusChristiNotEnglishThursday = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::June / 4,
    englishEpiphanyCalendar,
    "england");

if (corpusChristiNotEnglishThursday.observanceName ==
        "The Most Holy Body and Blood of Christ")
{
    std::cerr << "Corpus Christi incorrectly observed on Thursday in England\n";
    return 1;
}

auto sacredHeartDay = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::June / 12,
    celebrations,
    "grc");

if (sacredHeartDay.observanceName !=
        "The Most Sacred Heart of Jesus" ||
    sacredHeartDay.rank != CelebrationRank::Solemnity ||
    sacredHeartDay.precedence !=
        PrecedenceLevel::GeneralSolemnityOrAllSouls)
{
    std::cerr << "Sacred Heart resolution test failed\n";
    return 1;
}

auto sacredHeart2025 = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::June / 27,
    celebrations,
    "grc");

if (sacredHeart2025.observanceName !=
        "The Most Sacred Heart of Jesus" ||
    sacredHeart2025.rank != CelebrationRank::Solemnity ||
    sacredHeart2025.precedence !=
        PrecedenceLevel::GeneralSolemnityOrAllSouls)
{
    std::cerr << "Sacred Heart 2025 test failed\n";
    return 1;
}

auto christTheKing2026 = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::November / 22,
    celebrations,
    "grc");

if (christTheKing2026.observanceName !=
        "Our Lord Jesus Christ, King of the Universe" ||
    christTheKing2026.rank != CelebrationRank::Solemnity ||
    christTheKing2026.precedence !=
        PrecedenceLevel::GeneralSolemnityOrAllSouls)
{
    std::cerr << "Christ the King 2026 test failed\n";
    return 1;
}

auto christTheKing2025 = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::November / 23,
    celebrations,
    "grc");

if (christTheKing2025.observanceName !=
        "Our Lord Jesus Christ, King of the Universe" ||
    christTheKing2025.rank != CelebrationRank::Solemnity ||
    christTheKing2025.precedence !=
        PrecedenceLevel::GeneralSolemnityOrAllSouls)
{
    std::cerr << "Christ the King 2025 test failed\n";
    return 1;
}

auto allSoulsSunday = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::November / 2,
    celebrations,
    "grc");

if (allSoulsSunday.observanceName.find("Faithful Departed") ==
        std::string::npos ||
    allSoulsSunday.rank.has_value() ||
    allSoulsSunday.precedence !=
        PrecedenceLevel::GeneralSolemnityOrAllSouls)
{
    std::cerr << "All Souls Sunday test failed\n";
    return 1;
}

auto pentecostDay = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::May / 24,
    celebrations,
    "grc");

if (pentecostDay.observanceName != "Pentecost Sunday" ||
    pentecostDay.rank != CelebrationRank::Solemnity ||
    pentecostDay.precedence != PrecedenceLevel::HighestDays)
{
    std::cerr << "Pentecost resolution test failed\n";
    return 1;
}

auto ascensionDay = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::May / 14,
    celebrations,
    "grc");

if (ascensionDay.observanceName !=
        "The Ascension of the Lord" ||
    ascensionDay.rank != CelebrationRank::Solemnity ||
    ascensionDay.precedence != PrecedenceLevel::HighestDays)
{
    std::cerr << "Ascension resolution test failed\n";
    return 1;
}

auto easterSunday2026 = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::April / 5,
    celebrations,
    "grc");

if (easterSunday2026.observanceName !=
        "Easter Sunday of the Resurrection of the Lord" ||
    easterSunday2026.rank != CelebrationRank::Solemnity ||
    easterSunday2026.precedence != PrecedenceLevel::Triduum)
{
    std::cerr << "Easter Sunday resolution test failed\n";
    return 1;
}

auto easterMonday2026 = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::April / 6,
    celebrations,
    "grc");

if (easterMonday2026.observanceName !=
        "Monday within the Octave of Easter" ||
    easterMonday2026.season != LiturgicalPeriod::Easter ||
    easterMonday2026.precedence != PrecedenceLevel::HighestDays)
{
    std::cerr << "Easter Monday resolution test failed\n";
    return 1;
}

auto secondSundayOfEaster = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::April / 12,
    celebrations,
    "grc");

if (secondSundayOfEaster.observanceName !=
        "Second Sunday of Easter (or of Divine Mercy)" ||
    secondSundayOfEaster.season != LiturgicalPeriod::Easter ||
    secondSundayOfEaster.precedence != PrecedenceLevel::HighestDays)
{
    std::cerr << "Second Sunday of Easter test failed\n";
    return 1;
}

auto sixthSundayOfEaster = resolveLiturgicalDay(
    std::chrono::year{2025} / std::chrono::May / 25,
    celebrations,
    "grc");

if (sixthSundayOfEaster.observanceName !=
        "Sixth Sunday of Easter" ||
    sixthSundayOfEaster.season != LiturgicalPeriod::Easter ||
    sixthSundayOfEaster.precedence != PrecedenceLevel::HighestDays)
{
    std::cerr << "Sixth Sunday of Easter test failed\n";
    return 1;
}

auto secondWeekMonday = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::April / 13,
    celebrations,
    "grc");

if (secondWeekMonday.observanceName !=
        "Monday of the Second Week of Easter" ||
    secondWeekMonday.season != LiturgicalPeriod::Easter ||
    secondWeekMonday.precedence != PrecedenceLevel::OtherWeekday)
{
    std::cerr << "Second Week of Easter weekday test failed\n";
    return 1;
}

auto lastEasterWeekday = resolveLiturgicalDay(
    std::chrono::year{2026} / std::chrono::May / 23,
    celebrations,
    "grc");

if (lastEasterWeekday.observanceName !=
        "Saturday of the Seventh Week of Easter" ||
    lastEasterWeekday.season != LiturgicalPeriod::Easter ||
    lastEasterWeekday.precedence != PrecedenceLevel::OtherWeekday)
{
    std::cerr << "Last Easter weekday test failed\n";
    return 1;
}

std::cout << "Observance resolver tests passed\n";
	return 0;
}
