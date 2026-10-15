// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "movable_observances.h"
#include "temporal_calendar.h"
#include "easter.h"

#include <stdexcept>
#include <utility>

std::vector<ObservanceCandidate> findMovableObservances(
    std::chrono::year_month_day date, std::string_view calendarName)
{
    using namespace std::chrono;
    if (!date.ok()) throw std::invalid_argument("Invalid calendar date");
    const int year = int(date.year());
    const sys_days calendarDay{date};
    const sys_days pentecost{calculatePentecost(year)};
    std::vector<ObservanceCandidate> candidates;
    const auto addOn = [&](sys_days scheduled, std::string id, std::string name,
                           CelebrationRank rank, PrecedenceLevel precedence,
                           bool namesTemporalBaseline = false) {
        if (scheduled != calendarDay) return;
        ObservanceCandidate candidate;
        candidate.id = std::move(id);
        candidate.name = std::move(name);
        candidate.rank = rank;
        if (rank == CelebrationRank::Solemnity) candidate.subject = CelebrationSubject::Lord;
        candidate.precedence = precedence;
        candidate.optional = rank == CelebrationRank::OptionalMemorial;
        candidate.namesTemporalBaseline = namesTemporalBaseline;
        candidate.colour = candidate.id == "pentecost" ? LiturgicalColour::Red : LiturgicalColour::White;
        candidate.reducesMemorialCollision = candidate.id == "immaculate_heart";
        candidate.prevailsOnEqualPrecedence = candidate.id == "mary_mother_of_church" || candidate.id == "eternal_high_priest";
        candidates.push_back(std::move(candidate));
    };
    addOn(sys_days{calculateHolyFamily(year)}, "holy_family", "The Holy Family of Jesus, Mary and Joseph",
          CelebrationRank::Feast, PrecedenceLevel::GeneralFeastOfLord);
    addOn(sys_days{calculateBaptismOfLord(year, calendarName)}, "baptism_of_lord", "The Baptism of the Lord",
          CelebrationRank::Feast, PrecedenceLevel::GeneralFeastOfLord);
    addOn(pentecost + days{7}, "trinity_sunday", "The Most Holy Trinity",
          CelebrationRank::Solemnity, PrecedenceLevel::GeneralSolemnityOrAllSouls);
    const bool englandOrWales = calendarName == "england" || calendarName == "wales";
    addOn(pentecost + days{englandOrWales ? 14 : 11}, "corpus_christi", "The Most Holy Body and Blood of Christ",
          CelebrationRank::Solemnity, PrecedenceLevel::GeneralSolemnityOrAllSouls);
    addOn(pentecost + days{19}, "sacred_heart", "The Most Sacred Heart of Jesus",
          CelebrationRank::Solemnity, PrecedenceLevel::GeneralSolemnityOrAllSouls);
    addOn(sys_days{calculateFirstAdventSunday(year)} - days{7}, "christ_the_king",
          "Our Lord Jesus Christ, King of the Universe",
          CelebrationRank::Solemnity, PrecedenceLevel::GeneralSolemnityOrAllSouls);
    addOn(sys_days{calculateAscension(year)}, "ascension", "The Ascension of the Lord",
          CelebrationRank::Solemnity, PrecedenceLevel::HighestDays, true);
    addOn(pentecost, "pentecost", "Pentecost Sunday",
          CelebrationRank::Solemnity, PrecedenceLevel::HighestDays, true);
    addOn(sys_days{calculateEaster(year)}, "easter_sunday", "Easter Sunday of the Resurrection of the Lord",
          CelebrationRank::Solemnity, PrecedenceLevel::Triduum, true);
    addOn(pentecost + days{1}, "mary_mother_of_church", "The Blessed Virgin Mary, Mother of the Church",
          CelebrationRank::Memorial, PrecedenceLevel::GeneralMemorial);
    addOn(pentecost + days{20}, "immaculate_heart", "The Immaculate Heart of the Blessed Virgin Mary",
          CelebrationRank::Memorial, PrecedenceLevel::GeneralMemorial);
    if (englandOrWales)
        addOn(pentecost + days{4}, "eternal_high_priest", "Our Lord Jesus Christ, the Eternal High Priest",
              CelebrationRank::Feast, PrecedenceLevel::ProperFeast);
    if (isOrdinaryTime(date, calendarName) && weekday{calendarDay} == Saturday)
        addOn(calendarDay, "bvm_saturday", "The Blessed Virgin Mary on Saturday",
              CelebrationRank::OptionalMemorial, PrecedenceLevel::OptionalMemorial);
    return candidates;
}
