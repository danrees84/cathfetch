#include "movable_observances.h"
#include "temporal_calendar.h"
#include "easter.h"

#include <stdexcept>

std::vector<ObservanceCandidate> findMovableObservances(
    std::chrono::year_month_day date,
    std::string_view calendarName)
{
    if (!date.ok())
    {
        throw std::invalid_argument("Invalid calendar date");
    }

    std::vector<ObservanceCandidate> candidates;

    if (date == calculateHolyFamily(int(date.year())))
    {
        ObservanceCandidate holyFamily{
            "holy_family",
            "The Holy Family of Jesus, Mary and Joseph",
            CelebrationRank::Feast,
            PrecedenceLevel::GeneralFeastOfLord,
            false
        };

        candidates.push_back(holyFamily);
    }

if (date == calculateBaptismOfLord(
        int(date.year()), calendarName))
{
    ObservanceCandidate baptism{
        "baptism_of_lord",
        "The Baptism of the Lord",
        CelebrationRank::Feast,
        PrecedenceLevel::GeneralFeastOfLord,
        false
    };

    candidates.push_back(baptism);
}

    std::chrono::sys_days pentecostDay{
        calculatePentecost(int(date.year()))
    };

    std::chrono::year_month_day trinitySunday{
        pentecostDay + std::chrono::days{7}
    };

    if (date == trinitySunday)
    {
        ObservanceCandidate trinity{
            "trinity_sunday",
            "The Most Holy Trinity",
            CelebrationRank::Solemnity,
            PrecedenceLevel::GeneralSolemnityOrAllSouls,
            false
        };

        candidates.push_back(trinity);
    }

    std::chrono::sys_days trinityDate{trinitySunday};

    std::chrono::days daysAfterTrinity{4};

    if (calendarName == "england" || calendarName == "wales")
    {
        daysAfterTrinity = std::chrono::days{7};
    }

    std::chrono::year_month_day corpusChristiDate{
        trinityDate + daysAfterTrinity
    };

    if (date == corpusChristiDate)
    {
        ObservanceCandidate corpusChristi{
            "corpus_christi",
            "The Most Holy Body and Blood of Christ",
            CelebrationRank::Solemnity,
            PrecedenceLevel::GeneralSolemnityOrAllSouls,
            false
        };

        candidates.push_back(corpusChristi);
    }

    std::chrono::year_month_day sacredHeartDate{
        pentecostDay + std::chrono::days{19}
    };

    if (date == sacredHeartDate)
    {
        ObservanceCandidate sacredHeart{
            "sacred_heart",
            "The Most Sacred Heart of Jesus",
            CelebrationRank::Solemnity,
            PrecedenceLevel::GeneralSolemnityOrAllSouls,
            false
        };

        candidates.push_back(sacredHeart);
    }

    std::chrono::sys_days firstAdventDay{
        calculateFirstAdventSunday(int(date.year()))
    };

    std::chrono::year_month_day christTheKingDate{
        firstAdventDay - std::chrono::days{7}
    };

    if (date == christTheKingDate)
    {
        ObservanceCandidate christTheKing{
            "christ_the_king",
            "Our Lord Jesus Christ, King of the Universe",
            CelebrationRank::Solemnity,
            PrecedenceLevel::GeneralSolemnityOrAllSouls,
            false
        };

        candidates.push_back(christTheKing);
    }

    if (date == calculateAscension(int(date.year())))
    {
        ObservanceCandidate ascension{
            "ascension",
            "The Ascension of the Lord",
            CelebrationRank::Solemnity,
            PrecedenceLevel::HighestDays,
            false, true
        };

        candidates.push_back(ascension);
    }

    if (date == calculatePentecost(int(date.year())))
    {
        ObservanceCandidate pentecost{
            "pentecost",
            "Pentecost Sunday",
            CelebrationRank::Solemnity,
            PrecedenceLevel::HighestDays,
            false, true
        };

        candidates.push_back(pentecost);
    }

    if (date == calculateEaster(int(date.year())))
    {
        ObservanceCandidate easterSunday{
            "easter_sunday",
            "Easter Sunday of the Resurrection of the Lord",
            CelebrationRank::Solemnity,
            PrecedenceLevel::Triduum,
            false,
            true
        };

        candidates.push_back(easterSunday);
    }

    return candidates;
}
