#include <chrono>
#include <iostream>

#include "movable_observances.h"

int main()
{
    auto holyFamily2026 = findMovableObservances(
        std::chrono::year{2026} / std::chrono::December / 27);

    if (holyFamily2026.size() != 1 ||
        holyFamily2026.front().id != "holy_family" ||
        holyFamily2026.front().precedence !=
            PrecedenceLevel::GeneralFeastOfLord)
    {
        std::cerr << "Holy Family 2026 test failed\n";
        return 1;
    }

    auto holyFamily2022 = findMovableObservances(
        std::chrono::year{2022} / std::chrono::December / 30);

    if (holyFamily2022.size() != 1 ||
        holyFamily2022.front().id != "holy_family")
    {
        std::cerr << "Holy Family 2022 test failed\n";
        return 1;
    }

    auto december26 = findMovableObservances(
        std::chrono::year{2026} / std::chrono::December / 26);

    if (!december26.empty())
    {
        std::cerr << "Unexpected movable observance\n";
        return 1;
    }


struct BaptismTestCase
{
    std::chrono::year_month_day date;
    std::string_view calendarName;
};

const BaptismTestCase baptismTests[] = {
    {std::chrono::year{2024} / std::chrono::January / 7, "grc"},
    {std::chrono::year{2024} / std::chrono::January / 8, "england"},
    {std::chrono::year{2026} / std::chrono::January / 11, "wales"}
};

for (const auto& test : baptismTests)
{
    auto candidates = findMovableObservances(
        test.date, test.calendarName);

    if (candidates.size() != 1 ||
        candidates.front().id != "baptism_of_lord" ||
        candidates.front().precedence !=
            PrecedenceLevel::GeneralFeastOfLord)
    {
        std::cerr << "Baptism candidate test failed: "
                  << test.date << " ("
                  << test.calendarName << ")\n";
        return 1;
    }
}

auto englandJanuary7 = findMovableObservances(
    std::chrono::year{2024} / std::chrono::January / 7,
    "england");

if (!englandJanuary7.empty())
{
    std::cerr << "Baptism incorrectly generated on Epiphany\n";
    return 1;
}


    std::cout << "Movable observance tests passed\n";
    return 0;
}
