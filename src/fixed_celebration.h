#ifndef FIXED_CELEBRATION_H
#define FIXED_CELEBRATION_H

#include <chrono>
#include <string>
#include <optional>
#include "celebration_rank.h"
#include "liturgical_colour.h"

enum class PrecedenceLevel : int;

enum class FixedDateRule { Nominal, Epiphany, AdjacentSunday };
enum class SolemnityTransferRule { General, SaintJoseph, Annunciation, BeforeSacredHeart };

struct FixedCelebration
{
	std::string id;
	std::chrono::month_day date;
	std::string name;
	std::optional<CelebrationRank> rank;
std::string rankSourceCalendar = "general_roman";
bool feastOfLord = false;
    std::optional<PrecedenceLevel> precedence;
    bool namesTemporalBaseline = false;
    FixedDateRule dateRule = FixedDateRule::Nominal;
    SolemnityTransferRule transferRule = SolemnityTransferRule::General;
    std::optional<LiturgicalColour> colour;
};

#endif
