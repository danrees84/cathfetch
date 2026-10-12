#include "calendar_loader.h"

#include <fstream>
#include <optional>
#include <stdexcept>
#include <nlohmann/json.hpp>

std::vector<FixedCelebration> loadFixedCelebrations(
    const std::string& filename)
{
    std::ifstream file{filename};

    if (!file)
    {
        throw std::runtime_error(
            "Could not open calendar file: " + filename);
    }

    nlohmann::json calendar;
    file >> calendar;

    std::vector<FixedCelebration> celebrations;

    for (const auto& entry : calendar.at("entries"))
    {
        int month = entry.at("month").get<int>();
        int day = entry.at("day").get<int>();

        std::optional<std::string> rank;

        if (!entry.at("rank").is_null())
        {
            rank = entry.at("rank").get<std::string>();
        }

        FixedCelebration celebration{
            entry.at("id").get<std::string>(),
            std::chrono::month{unsigned(month)} /
                std::chrono::day{unsigned(day)},
            entry.at("name").get<std::string>(),
            rank
        };

        celebrations.push_back(celebration);
    }

    return celebrations;
}
