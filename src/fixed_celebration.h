#ifndef FIXED_CELEBRATION_H
#define FIXED_CELEBRATION_H

#include <chrono>
#include <string>
#include <optional>

struct FixedCelebration
{
    std::string id;
    std::chrono::month_day date;
    std::string name;
    std::optional<std::string> rank;
};

#endif
