// A date diagnostic exposes editorial gaps independently of reading availability.
#include "scripture_summary.h"
#include "lectionary_loader.h"
#include "lectionary_resolver.h"
#include "calendar_selection.h"
#include "liturgical_resolver.h"
#include <charconv>
#include <iostream>
#include <stdexcept>
int main(int argc, char** argv)
{
    try
    {
        if (argc < 2 || argc > 3) throw std::invalid_argument("Usage: summary_year YEAR [grc|europe|england|wales]");
        const std::string_view input{argv[1]};
        int y = 0;
        const auto parsed = std::from_chars(input.data(), input.data()+input.size(), y);
        if (parsed.ec != std::errc{} || parsed.ptr != input.data()+input.size() || y < 1583 || y > 4099)
            throw std::invalid_argument("Year must be 1583–4099");
        const std::string_view name = argc == 3 ? argv[2] : "grc";
        loadSelectedCalendar(name); // Validate the requested calendar before printing.
        const auto calendar = loadSelectedCalendar("grc");
        const auto lectionary = loadLectionary();
        const auto bible = loadBible();
        const auto summaries = loadScriptureSummaries(lectionary, bible);
        std::cout << "date\treading_set\tsummary\tcategory\treference_verification\teditorial_status\tpermitted_alternative\n";
        using namespace std::chrono;
        for (sys_days date = year{y}/January/1; date < sys_days{year{y+1}/January/1}; date += days{1})
        {
            const year_month_day civil{date};
            const auto reading = resolveMassReadings(resolveLiturgicalDay(civil, calendar, name), lectionary, name);
            const auto summary = resolveScriptureSummary(reading, summaries);
            std::cout << int(civil.year()) << '-' << unsigned(civil.month()) << '-' << unsigned(civil.day()) << '\t'
                      << (reading.selected ? reading.selected->id : "none") << '\t';
            if (summary.selected)
                std::cout << "available\t" << summary.selected->category << '\t' << summary.selected->referenceStatus
                          << '\t' << summary.selected->editorialStatus;
            else
                std::cout << (reading.status == ReadingStatus::NoDaytimeMass ? "no_daytime_mass" : "missing") << "\t-\t-\t-";
            std::cout << '\t' << (reading.selected && reading.selected->permittedAlternative ? "yes" : "no") << '\n';
        }
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
