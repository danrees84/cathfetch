#include "calendar_selection.h"
#include "liturgical_resolver.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
}

int main()
{
    using namespace std::chrono;
    try
    {
        for (const auto calendar : {"grc", "europe"})
        {
            auto definitions = loadSelectedCalendar(calendar);
            for (int order = 0; order < 2; ++order)
            {
                for (const auto y : {1943y, 2038y}) // 1943 is exclusively a modern-rule projection.
                {
                    const auto john = resolveLiturgicalDay(y/June/23, definitions, calendar);
                    const auto corpus = resolveLiturgicalDay(y/June/24, definitions, calendar);
                    const auto following = resolveLiturgicalDay(y/June/25, definitions, calendar);
                    require(john.observanceName.find("Nativity of St John") != std::string::npos, "St John anticipated, not omitted");
                    require(john.rank == CelebrationRank::Solemnity && int(john.precedence) == 3, "John retains solemnity and level 3");
                    require(john.transferredFrom == y/June/24 && john.provisionalTransfer && !john.transferNote.empty(), "Provisional transfer is explicit");
                    require(corpus.observanceId == "corpus_christi" && corpus.rank == CelebrationRank::Solemnity &&
                            int(corpus.precedence) == 3 && corpus.unresolvedReason.empty(), "Corpus Christi wins at existing level 3");
                    require(!corpus.provisionalTransfer && corpus.displacedObservances.size() == 1, "Only the transfer date is provisional");
                    const auto& displaced = corpus.displacedObservances.front();
                    require(displaced.id == john.observanceId && displaced.rank == CelebrationRank::Solemnity &&
                            int(displaced.precedence) == 3 && displaced.transferredTo == y/June/23 &&
                            displaced.provisionalTransfer, "Displaced solemnity and provisional destination retained");
                    require(following.observanceId == "temporal" && !following.transferredFrom &&
                            !following.provisionalTransfer, "June 25 retains ordinary calendar");
                }
                const auto john2022 = resolveLiturgicalDay(2022y/June/23, definitions, calendar);
                const auto heart2022 = resolveLiturgicalDay(2022y/June/24, definitions, calendar);
                require(john2022.transferredFrom == 2022y/June/24 && !john2022.provisionalTransfer &&
                        heart2022.observanceId == "sacred_heart", "2022 disposition remains confirmed");
                std::ranges::reverse(definitions);
            }
        }
        // Exercise the subject-based comparator with different IDs and a general
        // transfer policy: no hardcoded John/Corpus identity or class promotion.
        FixedCelebration saint;
        saint.id = "a-saint"; saint.name = "A saint"; saint.date = October/6;
        saint.rank = CelebrationRank::Solemnity; saint.subject = CelebrationSubject::Saint;
        FixedCelebration lord = saint;
        lord.id = "z-lord"; lord.name = "A solemnity of the Lord"; lord.subject = CelebrationSubject::Lord;
        for (auto definitions : {std::vector{saint, lord}, std::vector{lord, saint}})
        {
            const auto observed = resolveLiturgicalDay(2026y/October/6, definitions);
            require(observed.observanceId == lord.id && int(observed.precedence) == 3, "Semantic priority, independent of ID and vector order");
            const auto transferred = resolveLiturgicalDay(2026y/October/7, definitions);
            require(transferred.observanceId == saint.id && transferred.transferredFrom == 2026y/October/6,
                    "General transfer mechanism retains an impeded saint solemnity");
        }
        auto local = loadSelectedCalendar("grc");
        auto& patron = *std::ranges::find_if(local, [](const auto& definition) {
            return definition.transferRule == SolemnityTransferRule::AnticipateBeforeLord;
        });
        patron.principalPatron = true;
        // Proper classification must not bypass the explicit patronal exception.
        patron.rankSourceCalendar = "local";
        for (int order = 0; order < 2; ++order)
        {
            const auto heart = resolveLiturgicalDay(2022y/June/23, local);
            const auto john = resolveLiturgicalDay(2022y/June/24, local);
            require(heart.observanceId == "sacred_heart" && heart.transferredFrom == 2022y/June/24 &&
                    !heart.provisionalTransfer, "Patronal exception anticipates Sacred Heart");
            require(john.observanceName.find("Nativity of St John") != std::string::npos &&
                    !john.transferredFrom && !john.provisionalTransfer, "Principal patron remains on June 24");
            const auto pending = resolveLiturgicalDay(2038y/June/24, local);
            require(!pending.unresolvedReason.empty() && pending.conflictingObservances.size() == 2,
                    "2022 patronal exception does not authorise a 2038 local arrangement");
            std::ranges::reverse(local);
        }
        // Keep John at general class 3 to exercise the guarded equal-class rule.
        auto generalLocal = loadSelectedCalendar("grc");
        auto& generalPatron = *std::ranges::find_if(generalLocal, [](const auto& d) {
            return d.transferRule == SolemnityTransferRule::AnticipateBeforeLord;
        });
        generalPatron.principalPatron = true;
        const auto local2038 = resolveLiturgicalDay(2038y/June/24, generalLocal);
        require(!local2038.unresolvedReason.empty() && local2038.conflictingObservances.size() == 2,
                "Unspecified local patronal disposition is not invented for 2038");
        std::cout << "Solemnity occurrence tests passed\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
