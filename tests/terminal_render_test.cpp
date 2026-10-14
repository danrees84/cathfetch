#include "terminal_display.h"
#include "terminal_format.h"
#include "calendar_selection.h"
#include "liturgical_resolver.h"

#include <array>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
std::vector<std::string> lines(const std::string& text)
{
    std::istringstream stream{text};
    std::vector<std::string> result;
    for (std::string line; std::getline(stream, line);) result.push_back(line);
    return result;
}
std::string plain(const std::string& text)
{
    static const std::regex ansi{"\033\\[[0-?]*[ -/]*[@-~]"};
    return std::regex_replace(text, ansi, "");
}
}

int main()
{
    try
    {
        using namespace std::chrono;
        const auto calendar = loadSelectedCalendar("grc");
        const auto immaculate = resolveLiturgicalDay(2026y/December/8, calendar);
        const auto basil = resolveLiturgicalDay(2026y/January/2, calendar);
        const std::array<std::string, 8> cross{
            "          ███", "          ███", "      ███████████", "          ███",
            "          ███", "          ███", "          ███", "          ███"};
        for (const auto& day : {immaculate, basil})
            for (const auto width : {120u, 100u, 80u, 60u, 40u, 55u, 54u, 23u, 11u, 8u, 1u})
                for (const bool showCross : {false, true})
                {
                    const auto output = renderLiturgicalDay(day, showCross, width);
                    const auto rows = lines(output);
                    for (const auto& row : rows)
                        require(visibleTerminalWidth(row) <= width, "Rendered row exceeds terminal width");
                    if (showCross && width >= 55)
                    {
                        require(rows.size() >= cross.size(), "Cross height preserved");
                        for (std::size_t i = 0; i < cross.size(); ++i)
                            require(rows[i].starts_with(cross[i]), "Cross artwork preserved beside wrapped text");
                    }
                    if (showCross && width < 55 && width >= 11)
                    {
                        require(plain(rows[2]).find("███████████") != std::string::npos,
                                "Stacked cross preserves full arm");
                        require(rows[8].empty(), "Stacked artwork separated from information");
                    }
                    require(output.find("\033[35m●\033[0m Advent") != std::string::npos ||
                            day.season != LiturgicalPeriod::Advent || width < 10, "Season colour retained");
                }
        const auto wide = plain(renderLiturgicalDay(immaculate, true, 120));
        require(wide.find(cross[1] + "          Tuesday, 8 December 2026") != std::string::npos,
                "Original right-hand column preserved");
        const auto narrow = lines(plain(renderLiturgicalDay(immaculate, true, 60)));
        // A continuation after a two-cell bullet starts at column 25 (zero-based).
        require(narrow[4].starts_with("          ███            "), "Hanging continuation indentation");
        auto unusuallyLong = immaculate;
        unusuallyLong.observanceName.clear();
        for (int i = 0; i < 80; ++i) unusuallyLong.observanceName += "Long celebration ";
        const auto extended = lines(plain(renderLiturgicalDay(unusuallyLong, true, 60)));
        require(extended.size() > cross.size(), "Text may extend below the cross");
        for (std::size_t i = cross.size(); i < extended.size(); ++i)
            require(extended[i].starts_with(std::string(25, ' ')) && visibleTerminalWidth(extended[i]) <= 60,
                    "Rows below artwork retain the hanging text column");
        const auto ordinary = lines(plain(renderLiturgicalDay(immaculate, false, 40)));
        require(ordinary[3].starts_with("  "), "No-cross continuation accounts for bullet");

        require(visibleTerminalWidth("\033[31m●\033[0m St Thérèse") == 12, "ANSI-free Unicode cell count");
        require(visibleTerminalWidth("e\xcc\x81") == 1, "Combining accent has zero additional width");
        require(visibleTerminalWidth("聖人") == 4, "Wide characters occupy two cells each");
        require(visibleTerminalWidth("\033]8;;https://example.org\033\\聖人\033]8;;\033\\") == 4,
                "OSC hyperlink escapes occupy no cells");
        const auto combining = wrapTerminalText("e\xcc\x81" "e\xcc\x81", 1);
        require(combining.size() == 2 && combining[0] == "e\xcc\x81" && combining[1] == "e\xcc\x81",
                "Wrapping preserves combining clusters");
        require(visibleTerminalWidth("👩‍💻") == 2, "Common joined emoji occupies two cells");
        require(wrapTerminalText("👩‍💻👩‍💻", 2) == std::vector<std::string>{"👩‍💻", "👩‍💻"},
                "Joined emoji is not split across lines");
        require(visibleTerminalWidth("聖‍人") == 4, "A joiner does not collapse unrelated wide characters");
        const auto cjk = wrapTerminalText("聖人聖人", 4);
        require(cjk == std::vector<std::string>{"聖人", "聖人"}, "Wrapping preserves UTF-8 code points");
        const auto longWord = wrapTerminalText(std::string(101, 'a'), 16);
        std::string joined;
        for (const auto& row : longWord) { require(visibleTerminalWidth(row) <= 16, "Long word width"); joined += row; }
        require(joined == std::string(101, 'a'), "Long words split without truncation");
        const auto coloured = wrapTerminalText("\033[31mabcdef\033[0m", 3);
        require(coloured.size() == 2 && visibleTerminalWidth(coloured[0]) == 3 &&
                visibleTerminalWidth(coloured[1]) == 3, "ANSI sequences survive hard wrapping");
        const auto malformed = wrapTerminalText(std::string{"bad\xfftext"}, 4);
        for (const auto& row : malformed) require(row.find('\xff') == std::string::npos, "Malformed UTF-8 replaced safely");
        require(wrapTerminalText("one two three", 7, 2) == std::vector<std::string>{"one two", "  three"},
                "Wrapping prefers whole words");
        bool rejected = false;
        try { renderLiturgicalDay(immaculate, true, 0); } catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, "Zero width rejected");
        std::cout << "Terminal rendering tests passed\n";
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
