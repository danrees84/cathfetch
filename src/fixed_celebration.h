#ifndef FIXED_CELEBRATION_H
#define FIXED_CELEBRATION_H

#include <chrono>
#include <string>
#include <optional>
#include "celebration_rank.h"

struct FixedCelebration
{
	std::string id;
	std::chrono::month_day date;
	std::string name;
	std::optional<CelebrationRank> rank;
std::string rankSourceCalendar = "general_roman";
bool feastOfLord = false;
};

#endif
