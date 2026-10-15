// Start here when reading Cathfetch: parse the options, resolve the day, then
// render it. Calendar, reading and Scripture rules live in their own modules.
#include "calendar_selection.h"
#include "lectionary_loader.h"
#include "lectionary_resolver.h"
#include "liturgical_resolver.h"
#include "scripture.h"
#include "terminal_display.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
// These helpers belong only to the command-line application. Keeping them in
// an unnamed namespace prevents other source files from depending on them.
std::chrono::year_month_day currentLocalDate()
{
    const std::time_t now = std::time(nullptr);
    const std::tm* local = std::localtime(&now);
    if (local == nullptr)
    {
        throw std::runtime_error("Could not retrieve local date");
    }

    // The C time structure counts years from 1900 and months from zero.
    return {std::chrono::year{local->tm_year + 1900},
            std::chrono::month{unsigned(local->tm_mon + 1)},
            std::chrono::day{unsigned(local->tm_mday)}};
}

int parseDigits(std::string_view digits)
{
    // parseDate checks that every character is a digit before calling this.
    int number = 0;
    for (char digit : digits)
    {
        number = number * 10 + (digit - '0');
    }
    return number;
}

std::chrono::year_month_day parseDate(std::string_view value)
{
    const bool allDigits = std::ranges::all_of(value, [](char character) {
        return character >= '0' && character <= '9';
    });
    if (value.size() != 8 || !allDigits)
    {
        throw std::invalid_argument("Date must be DDMMYYYY");
    }

    const int day = parseDigits(value.substr(0, 2));
    const int month = parseDigits(value.substr(2, 2));
    const int year = parseDigits(value.substr(4, 4));
    const std::chrono::year_month_day date{
        std::chrono::year{year}, std::chrono::month{unsigned(month)}, std::chrono::day{unsigned(day)}};
    if (!date.ok())
    {
        throw std::invalid_argument("Invalid calendar date");
    }
    return date;
}

void printHelp()
{
    // Plain text goes to stdout so help can be piped to a pager or saved.
    std::cout << R"(Usage: cathfetch [OPTIONS]
Display the Catholic liturgical day for a date and calendar.

Options:
  -h, --help             Show this help and exit.
  --date=DDMMYYYY        Choose a date (1583-4099; default: today, local time).
  --calendar=NAME        grc, europe, england or wales (default: grc).
  --cross                Display the cross beside or above the information.
  --readings             Show universal Mass reading references (v1 policy).
  --summary              Show a prewritten Scripture quotation and commentary.
  --full-verses          Show offline Douay-Rheims verses; implies --readings.
  --mass=FORM            Choose a Mass form; implies --readings.
                         Forms: day, vigil, vigil-extended, night, dawn,
                         evening, passion. Only available forms can be used.
  --                     End options. No positional arguments are accepted.

Options with values also accept '--option VALUE'.
Without --mass, the default form is selected, usually the daytime Mass.
V1 readings omit memorial and national saints' propers, while retaining local
scheduling for universal celebrations. The displayed calendar stays regional.

Examples:
  cathfetch --date=08122026 --cross
  cathfetch --date=30112025 --summary
  cathfetch --calendar wales --readings
  cathfetch --date=25122026 --mass=night --full-verses
  cathfetch --date=04042026 --mass=vigil

Data is found beside the installation or development build, independently of
the working directory. CATHFETCH_DATA_DIR overrides the data directory.
)";
}

std::string_view optionValue(std::string_view argument, std::string_view name,
                             int& index, int argc, char* argv[])
{
    std::string_view value;
    if (argument.size() > name.size())
    {
        // The caller has already recognised the exact '--name=' prefix.
        value = argument.substr(name.size() + 1);
    }
    else if (index + 1 < argc)
    {
        // Consume the following argv element for the '--name value' spelling.
        value = argv[++index];
    }
    if (value.empty() || value.starts_with('-'))
    {
        throw std::invalid_argument("Missing value for " + std::string{name});
    }
    return value;
}

struct Options
{
    std::chrono::year_month_day date;
    std::string_view calendar = "grc";
    std::string_view mass = "default";
    bool showHelp = false;
    bool showCross = false;
    bool showReadings = false;
    bool fullVerses = false;
    bool showSummary = false;
    bool massSpecified = false;
};

Options parseOptions(int argc, char* argv[])
{
    Options options;
    bool dateSpecified = false;
    bool calendarSpecified = false;

    for (int index = 1; index < argc; ++index)
    {
        // string_view borrows argv's text; argv stays alive throughout main.
        const std::string_view argument{argv[index]};
        if (argument == "--help" || argument == "-h")
        {
            options.showHelp = true;
            return options;
        }
        if (argument == "--")
        {
            // There are no file operands: anything after '--' is an error,
            // even if it happens to look like another option.
            if (index + 1 < argc)
            {
                throw std::invalid_argument("Unexpected argument: " + std::string{argv[index + 1]});
            }
            break;
        }
        if (argument == "--cross")
        {
            if (options.showCross) throw std::invalid_argument("Duplicate option: --cross");
            options.showCross = true;
        }
        else if (argument == "--readings")
        {
            if (options.showReadings) throw std::invalid_argument("Duplicate option: --readings");
            options.showReadings = true;
        }
        else if (argument == "--full-verses")
        {
            if (options.fullVerses) throw std::invalid_argument("Duplicate option: --full-verses");
            options.fullVerses = true;
        }
        else if (argument == "--summary")
        {
            if (options.showSummary) throw std::invalid_argument("Duplicate option: --summary");
            options.showSummary = true;
        }
        else if (argument == "--mass" || argument.starts_with("--mass="))
        {
            if (options.massSpecified) throw std::invalid_argument("Duplicate option: --mass");
            options.mass = optionValue(argument, "--mass", index, argc, argv);
            if (!isMassForm(options.mass))
            {
                throw std::invalid_argument("Unknown Mass form: " + std::string{options.mass});
            }
            options.massSpecified = true;
        }
        else if (argument == "--date" || argument.starts_with("--date="))
        {
            if (dateSpecified) throw std::invalid_argument("Duplicate option: --date");
            options.date = parseDate(optionValue(argument, "--date", index, argc, argv));
            dateSpecified = true;
        }
        else if (argument == "--calendar" || argument.starts_with("--calendar="))
        {
            if (calendarSpecified) throw std::invalid_argument("Duplicate option: --calendar");
            options.calendar = optionValue(argument, "--calendar", index, argc, argv);
            if (options.calendar != "grc" && options.calendar != "europe" &&
                options.calendar != "england" && options.calendar != "wales")
            {
                throw std::invalid_argument("Unknown calendar: " + std::string{options.calendar});
            }
            calendarSpecified = true;
        }
        else
        {
            const auto description = argument.starts_with('-') ? "Unknown option: " : "Unexpected argument: ";
            throw std::invalid_argument(description + std::string{argument});
        }
    }
    // Help must work without consulting the clock or loading any data files.
    if (!dateSpecified) options.date = currentLocalDate();
    return options;
}
} // namespace

int main(int argc, char* argv[])
{
    try
    {
        const auto options = parseOptions(argc, argv);
        if (options.showHelp)
        {
            printHelp();
            return 0;
        }
        const auto celebrations = loadSelectedCalendar(options.calendar);
        const auto day = resolveLiturgicalDay(options.date, celebrations, options.calendar);
        const bool displayReadings = options.showReadings || options.massSpecified || options.fullVerses;
        const bool needsReadings = displayReadings || options.showSummary;
        if (!needsReadings)
        {
            printLiturgicalDay(day, options.showCross);
            return 0;
        }

        // V1 displays the regional celebration, but selects universal readings.
        // Reusing the calendar engine preserves local dates for universal feasts.
        const auto universalCalendar = loadSelectedCalendar("grc");
        const auto readingDay = resolveLiturgicalDay(options.date, universalCalendar, options.calendar);
        const auto lectionary = loadLectionary();
        const auto readings = resolveMassReadings(readingDay, lectionary, options.calendar, options.mass);
        // Summary-only output still uses the existing reading selection, but
        // does not implicitly print the list of readings or full passages.
        std::optional<Bible> bible;
        std::optional<ScriptureSummaryResult> summary;
        if (readings.status == ReadingStatus::Available && (options.fullVerses || options.showSummary))
            bible = loadBible();
        if (options.showSummary)
        {
            if (bible)
            {
                summary = loadScriptureSummary(readings, lectionary, *bible);
            }
            else
            {
                // A day without a Mass needs a clear message, not Scripture
                // files or editorial data which cannot be used on that day.
                summary = resolveScriptureSummary(readings, ScriptureSummaries{});
            }
        }
        printLiturgicalDay(day, options.showCross, displayReadings ? &readings : nullptr,
                          options.fullVerses && bible ? &*bible : nullptr, summary ? &*summary : nullptr);
    }
    catch (const std::invalid_argument& error)
    {
        std::cerr << "cathfetch: " << error.what() << "\nTry 'cathfetch --help' for usage.\n";
        return 1;
    }
    catch (const std::exception& error)
    {
        // Helpers report errors with exceptions; only main decides the exit code.
        std::cerr << "cathfetch: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
