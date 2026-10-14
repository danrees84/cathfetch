#include "calendar_selection.h"
#include "liturgical_resolver.h"
#include "lectionary_loader.h"
#include "lectionary_resolver.h"
#include <charconv>
#include <iostream>
#include <map>
#include <stdexcept>

int main(int argc, char** argv)
{
    try
    {
        if (argc < 2 || argc > 3)
            throw std::invalid_argument("Usage: lectionary_year YEAR [grc|europe|england|wales]");
        const std::string_view input{argv[1]};
        int year = 0;
        const auto parsed = std::from_chars(input.data(), input.data() + input.size(), year);
        if (parsed.ec != std::errc{} || parsed.ptr != input.data() + input.size() || year < 1583 || year > 4099)
            throw std::invalid_argument("Year must be 1583–4099");
        const std::string_view calendar = argc == 3 ? argv[2] : "grc";
        const auto celebrations = loadSelectedCalendar(calendar);
        const auto data = loadLectionary();
        using namespace std::chrono;
        std::map<ReadingStatus, int> counts;
        int verified = 0;
        int provisional = 0;
        int permittedAlternative = 0;
        int alternativesAvailable = 0;
        int dates = 0;
        std::cout << "date\tobservance\tsunday_cycle\tweekday_cycle\tstatus\tkey\tmass\treadings\tdiagnostic"
                     "\tverification\tpermitted_alternative_selected\talternatives_available\tsource\tsource_note"
                     "\tverification_sources\tverification_scope\n";
        for (sys_days date{std::chrono::year{year}/January/1};
             date < sys_days{std::chrono::year{year+1}/January/1}; date += days{1})
        {
            const auto day = resolveLiturgicalDay(year_month_day{date}, celebrations, calendar);
            const auto readings = resolveMassReadings(day, data, calendar);
            ++counts[readings.status];
            ++dates;
            const auto status = readings.status == ReadingStatus::Available ? "available" :
                readings.status == ReadingStatus::NoDaytimeMass ? "no-daytime-Mass" :
                readings.status == ReadingStatus::UnresolvedDay ? "unresolved-calendar" : "missing";
            std::cout << day.date << '\t' << day.observanceName << '\t'
                      << cycleName(readings.cycles.sunday) << '\t' << cycleName(readings.cycles.weekday)
                      << '\t' << status << '\t' << readings.key << '\t';
            if (readings.selected) std::cout << readings.selected->mass;
            std::cout << '\t';
            if (readings.selected)
                for (const auto& reading : readings.selected->readings)
                    std::cout << reading.role << ": " << reading.reference << "; ";
            int options = int(readings.alternatives.size());
            if (readings.selected)
                for (const auto& reading : readings.selected->readings)
                    options += int(reading.alternatives.size());
            if (options) ++alternativesAvailable;
            const bool optionalSelection = readings.selected &&
                (readings.selected->permittedAlternative || readings.selected->alternativeOnly ||
                 readings.selected->use == ReadingUse::Accommodated);
            if (optionalSelection) ++permittedAlternative;
            std::cout << '\t' << readings.diagnostic << '\t';
            if (readings.selected)
            {
                const bool current = readings.selected->verification == ReferenceVerification::CurrentEdition;
                if (current) ++verified;
                else ++provisional;
                std::cout << (current ? "current_edition" : "published_table");
            }
            else std::cout << "none";
            std::cout << '\t' << (optionalSelection ? "yes" : "no") << '\t' << options << '\t';
            if (readings.selected) std::cout << readings.selected->source;
            std::cout << '\t';
            if (readings.selected) std::cout << readings.selected->sourceNote;
            std::cout << '\t';
            if (readings.selected)
                for (const auto& source : readings.selected->verificationSources) std::cout << source << "; ";
            std::cout << '\t';
            if (readings.selected) std::cout << readings.selected->verificationNote;
            std::cout << '\n';
        }
        std::cerr << year << ' ' << calendar << ": " << dates << " dates; "
                  << counts[ReadingStatus::Available] << " with references; " << verified
                  << " current-edition verified; " << provisional << " provisionally sourced; "
                  << permittedAlternative << " using permitted alternatives; " << alternativesAvailable
                  << " with alternatives available; " << counts[ReadingStatus::MissingData] << " missing; "
                  << counts[ReadingStatus::NoDaytimeMass] << " without daytime Mass; "
                  << counts[ReadingStatus::UnresolvedDay] << " unresolved calendar days; "
                  << provisional << " selected assignments awaiting authoritative confirmation\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
