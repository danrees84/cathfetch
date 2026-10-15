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

struct Options
{
    std::chrono::year_month_day date;
    std::string_view calendar = "grc";
    std::string_view mass = "default";
    bool showCross = false;
    bool showReadings = false;
    bool fullVerses = false;
    bool massSpecified = false;
};

Options parseOptions(int argc, char* argv[])
{
    Options options;
    options.date = currentLocalDate();
    bool dateSpecified = false;
    bool calendarSpecified = false;

    for (int index = 1; index < argc; ++index)
    {
        // string_view borrows argv's text; argv stays alive throughout main.
        const std::string_view argument{argv[index]};
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
        else if (argument.starts_with("--mass="))
        {
            if (options.massSpecified) throw std::invalid_argument("Duplicate option: --mass");
            options.mass = argument.substr(7);
            if (!isMassForm(options.mass))
            {
                throw std::invalid_argument("Unknown Mass form: " + std::string{options.mass});
            }
            options.massSpecified = true;
        }
        else if (argument.starts_with("--date="))
        {
            if (dateSpecified) throw std::invalid_argument("Duplicate option: --date");
            options.date = parseDate(argument.substr(7));
            dateSpecified = true;
        }
        else if (argument.starts_with("--calendar="))
        {
            if (calendarSpecified) throw std::invalid_argument("Duplicate option: --calendar");
            options.calendar = argument.substr(11);
            if (options.calendar != "grc" && options.calendar != "europe" &&
                options.calendar != "england" && options.calendar != "wales")
            {
                throw std::invalid_argument("Unknown calendar: " + std::string{options.calendar});
            }
            calendarSpecified = true;
        }
        else
        {
            throw std::invalid_argument("Unknown option: " + std::string{argument});
        }
    }
    return options;
}
} // namespace

int main(int argc, char* argv[])
{
    try
    {
        const auto options = parseOptions(argc, argv);
        const auto celebrations = loadSelectedCalendar(options.calendar);
        const auto day = resolveLiturgicalDay(options.date, celebrations, options.calendar);
        const bool needsReadings = options.showReadings || options.massSpecified || options.fullVerses;
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
        if (options.fullVerses && readings.status == ReadingStatus::Available)
        {
            const auto bible = loadBible();
            printLiturgicalDay(day, options.showCross, &readings, &bible);
        }
        else
        {
            printLiturgicalDay(day, options.showCross, &readings);
        }
    }
    catch (const std::exception& error)
    {
        // Helpers report errors with exceptions; only main decides the exit code.
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
