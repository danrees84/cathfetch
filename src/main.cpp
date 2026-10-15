#include <iostream>
#include <chrono>
#include <ctime>
#include <stdexcept>
// #include "easter.h"
#include "temporal_calendar.h"
#include "liturgical_format.h"
#include "liturgical_colour.h"
#include "terminal_format.h"
#include "terminal_display.h"
#include <string_view>
#include <algorithm>
#include "calendar_selection.h"
#include "liturgical_resolver.h"
#include "celebration_rank.h"
#include "lectionary_loader.h"
#include "lectionary_resolver.h"

std::chrono::year_month_day getCurrentLocalDate()
{

	std::time_t now = std::time(nullptr);
	std::tm* local = std::localtime(&now);

	if (local == nullptr)
	{
		throw std::runtime_error("Could not retrieve local date");
	}

	std::chrono::year_month_day localDate{
		std::chrono::year{local->tm_year + 1900},
			std::chrono::month{unsigned(local->tm_mon + 1)},
			std::chrono::day{unsigned(local->tm_mday)}
	};

	return localDate;

}

int main(int argc, char* argv[])
{

	bool showCross = false;
    bool showReadings = false;
    bool fullVerses = false;
    std::string_view mass = "default";
    bool massSpecified = false;
	auto selectedDate = getCurrentLocalDate();
	bool dateSpecified = false;
	std::string_view selectedCalendar = "grc";
	bool calendarSpecified = false;

	for (int i = 1; i < argc; ++i)
	{
		std::string_view arg{argv[i]};

		if (arg == "--cross")
		{
			if (showCross)
			{
				std::cerr << "Duplicate option: --cross\n";
				return 1;
			}

			showCross = true;
		}

        else if (arg == "--readings")
        {
            if (showReadings) { std::cerr << "Duplicate option: --readings\n"; return 1; }
            showReadings = true;
        }
        else if (arg == "--full-verses")
        {
            if (fullVerses) { std::cerr << "Duplicate option: --full-verses\n"; return 1; }
            fullVerses = true;
        }
        else if (arg.starts_with("--mass="))
        {
            if (massSpecified) { std::cerr << "Duplicate option: --mass\n"; return 1; }
            mass = arg.substr(7);
            if (!isMassForm(mass))
            {
                std::cerr << "Unknown Mass form: " << mass << '\n';
                return 1;
            }
            massSpecified = true;
        }
		else if (arg.starts_with("--date="))
		{
			if (dateSpecified)
			{
				std::cerr << "Duplicate option: --date\n";
				return 1;
			}

			std::string_view value = arg.substr(7);

			if (value.size() != 8 ||
					!std::ranges::all_of(value, [](char c) {
						return c >= '0' && c <= '9';
						}))
			{
				std::cerr << "Date must be DDMMYYYY\n";
				return 1;
			}

			auto parseNumber = [](std::string_view digits)
			{
				int number = 0;

				for (char digit : digits)
				{
					number = number * 10 + (digit - '0');
				}

				return number;
			};

			int day = parseNumber(value.substr(0, 2));
			int month = parseNumber(value.substr(2, 2));
			int year = parseNumber(value.substr(4, 4));

			std::chrono::year_month_day parsedDate{
				std::chrono::year{year},
					std::chrono::month{unsigned(month)},
					std::chrono::day{unsigned(day)}
			};

			if (!parsedDate.ok())
			{
				std::cerr << "Invalid calendar date\n";
				return 1;
			}

			selectedDate = parsedDate;
			dateSpecified = true;


		}


		else if (arg.starts_with("--calendar="))
		{
			if (calendarSpecified)
			{
				std::cerr << "Duplicate option: --calendar\n";
				return 1;
			}

			auto calendarName = arg.substr(11);

			if (calendarName != "grc" &&
					calendarName != "europe" &&
					calendarName != "england" &&
					calendarName != "wales")
			{
				std::cerr << "Unknown calendar: " << calendarName << "\n";
				return 1;
			}

			selectedCalendar = calendarName;
			calendarSpecified = true;
		}

		else
		{
			std::cerr << "Unknown option: " << arg << "\n";
			return 1;
		}
	}




    try
    {
        const auto celebrations = loadSelectedCalendar(selectedCalendar);
        const auto day = resolveLiturgicalDay(selectedDate, celebrations, selectedCalendar);
        if (showReadings || massSpecified || fullVerses)
        {
            const auto data = loadLectionary();
            const auto readingDay = resolveLiturgicalDay(selectedDate, loadSelectedCalendar("grc"), selectedCalendar);
            const auto readings = resolveMassReadings(readingDay, data, selectedCalendar, mass);
            if (fullVerses && readings.status == ReadingStatus::Available)
            {
                const auto bible = loadBible();
                printLiturgicalDay(day, showCross, &readings, &bible);
            }
            else printLiturgicalDay(day, showCross, &readings);
        }
        else printLiturgicalDay(day, showCross);
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
