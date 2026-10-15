#ifndef FIXED_CELEBRATION_H
#define FIXED_CELEBRATION_H

#include <chrono>
#include <string>
#include <optional>
#include "celebration_rank.h"
#include "liturgical_colour.h"

enum class PrecedenceLevel : int;

enum class CelebrationSubject { Other, Lord, Saint };

enum class FixedDateRule { Nominal, Epiphany, AdjacentSunday, AllSouls };
enum class SolemnityTransferRule { General, SaintJoseph, Annunciation, AfterEasterOctave, AnticipateBeforeLord };

// A calendar definition stores the usual month/day. The resolver works out the
// actual date for a particular year, including scheduling rules and transfers.
struct FixedCelebration
{
    std::string id;
    std::chrono::month_day date;
    std::string name;
    std::optional<CelebrationRank> rank = std::nullopt;
    std::string rankSourceCalendar = "general_roman";
    bool feastOfLord = false;
    CelebrationSubject subject = CelebrationSubject::Other;
    bool principalPatron = false;
    std::optional<PrecedenceLevel> precedence = std::nullopt;
    bool namesTemporalBaseline = false;
    FixedDateRule dateRule = FixedDateRule::Nominal;
    SolemnityTransferRule transferRule = SolemnityTransferRule::General;
    std::optional<LiturgicalColour> colour = std::nullopt;
};

#endif
