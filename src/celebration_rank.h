#ifndef CELEBRATION_RANK_H
#define CELEBRATION_RANK_H

#include <string_view>
#include <optional>

enum class CelebrationRank
{
    Solemnity,
    Feast,
    Memorial,
    OptionalMemorial
};

CelebrationRank parseCelebrationRank(std::string_view value);
std::string_view celebrationRankName(CelebrationRank rank);
std::string_view celebrationRankName(
    std::optional<CelebrationRank> rank);

#endif
