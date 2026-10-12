#include <iostream>
#include "fixed_celebration.h"
#include "celebration_rank.h"

int main()
{
    FixedCelebration christmas{
        "grc-12-25-the-nativity-of-the-lord",
        std::chrono::December / std::chrono::day{25},
        "The Nativity of the Lord",
          CelebrationRank::Solemnity  };

    std::cout << christmas.name << "\n";
    std::cout << christmas.date << "\n";
std::cout << celebrationRankName(christmas.rank) << "\n";
    return 0;
}
