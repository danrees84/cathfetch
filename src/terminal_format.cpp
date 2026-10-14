#include "terminal_format.h"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <cwchar>
#include <locale.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>


std::string terminalColourCode(LiturgicalColour colour)
{
    switch (colour)
    {
        case LiturgicalColour::Green:
            return "\033[32m";

        case LiturgicalColour::Violet:
            return "\033[35m";

        case LiturgicalColour::Rose:
            return "\033[35m";

        case LiturgicalColour::Red:
            return "\033[31m";

        case LiturgicalColour::White:
            return "\033[37m";

        case LiturgicalColour::Black:
            return "\033[90m";
    }

    return "\033[0m";
}

namespace
{
// wcwidth uses the calling thread's character locale. Keep UTF-8 width handling
// independent of main's locale, without changing process-wide date formatting.
class CharacterLocale
{
public:
    CharacterLocale()
    {
        static const auto utf8 = newlocale(LC_CTYPE_MASK, "C.UTF-8", nullptr);
        if (utf8) previous = uselocale(utf8);
    }
    ~CharacterLocale() { if (previous) uselocale(previous); }
    CharacterLocale(const CharacterLocale&) = delete;
    CharacterLocale& operator=(const CharacterLocale&) = delete;
private:
    locale_t previous = nullptr;
};

struct Cell
{
    std::string text;
    std::size_t width;
    bool space = false;
    bool newline = false;
    bool emoji = false;
};

std::size_t escapeLength(std::string_view text)
{
    if (text.size() < 2 || text[0] != '\033') return 0;
    if (text[1] == '[')
        for (std::size_t i = 2; i < text.size(); ++i)
            if (text[i] >= '@' && text[i] <= '~') return i + 1;
    if (text[1] == ']')
        for (std::size_t i = 2; i < text.size(); ++i)
        {
            if (text[i] == '\a') return i + 1;
            if (text[i] == '\033' && i + 1 < text.size() && text[i + 1] == '\\') return i + 2;
        }
    return 0;
}

// Decode explicitly so malformed UTF-8 never causes a partial multibyte write.
std::pair<char32_t, std::size_t> decode(std::string_view text)
{
    const auto first = static_cast<unsigned char>(text[0]);
    if (first < 0x80) return {first, 1};
    const std::size_t count = first >= 0xc2 && first <= 0xdf ? 2 :
                              first >= 0xe0 && first <= 0xef ? 3 :
                              first >= 0xf0 && first <= 0xf4 ? 4 : 0;
    if (count == 0 || text.size() < count) return {0xfffd, 1};
    char32_t code = first & (0x7f >> count);
    for (std::size_t i = 1; i < count; ++i)
    {
        const auto byte = static_cast<unsigned char>(text[i]);
        if ((byte & 0xc0) != 0x80) return {0xfffd, 1};
        code = (code << 6) | (byte & 0x3f);
    }
    if ((count == 2 && code < 0x80) || (count == 3 && code < 0x800) ||
        (count == 4 && code < 0x10000) || code > 0x10ffff ||
        (code >= 0xd800 && code <= 0xdfff)) return {0xfffd, 1};
    return {code, count};
}

std::vector<Cell> cells(std::string_view text)
{
    CharacterLocale locale;
    std::vector<Cell> result;
    bool joinNext = false;
    bool regional = false;
    for (std::size_t offset = 0; offset < text.size();)
    {
        const auto rest = text.substr(offset);
        if (const auto length = escapeLength(rest))
        {
            // Preserve SGR colours and OSC hyperlinks, but count no cells.
            result.push_back({std::string{rest.substr(0, length)}, 0});
            offset += length;
            continue;
        }
        const auto [code, length] = decode(rest);
        offset += length;
        if (code == '\n' || code == ' ' || code == '\t')
        {
            result.push_back({code == '\n' ? "\n" : " ", code == '\n' ? 0u : 1u,
                              code != '\n', code == '\n'});
            joinNext = regional = false;
            continue;
        }
        const int measured = wcwidth(static_cast<wchar_t>(code));
        const std::size_t width = measured < 0 ? 1 : static_cast<std::size_t>(measured);
        const std::string bytes = measured < 0 || code == 0xfffd
            ? "\xef\xbf\xbd" : std::string{rest.substr(0, length)};
        const bool emoji = (code >= 0x1f000 && code <= 0x1faff) || (code >= 0x2600 && code <= 0x27bf);
        const bool isRegional = code >= 0x1f1e6 && code <= 0x1f1ff;
        // Keep combining marks, common emoji joiners/modifiers and flag pairs
        // together when a word has to break. CJK widths come from wcwidth.
        auto previous = result.end();
        while (previous != result.begin())
        {
            --previous;
            if (previous->width || previous->space || previous->newline) break;
        }
        const bool canJoin = previous != result.end() && !previous->space && !previous->newline && previous->width;
        if (canJoin && (width == 0 || (joinNext && previous->emoji && emoji) || (regional && isRegional) ||
                        (previous->emoji && code >= 0x1f3fb && code <= 0x1f3ff)))
        {
            // Include any intervening zero-width ANSI sequences in the cluster.
            for (auto after = previous + 1; after != result.end(); ++after) previous->text += after->text;
            result.erase(previous + 1, result.end());
            previous->text += bytes;
            if (regional && isRegional) previous->width = 2;
            else previous->width = std::max(previous->width, width);
            if (code == 0xfe0f) previous->width = std::max(previous->width, std::size_t{2});
        }
        else result.push_back({bytes, width, false, false, emoji});
        joinNext = code == 0x200d;
        regional = isRegional && !regional;
    }
    return result;
}
} // namespace

std::size_t terminalColumns()
{
    winsize size{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col > 0)
        return size.ws_col;
    if (const char* value = std::getenv("COLUMNS"))
    {
        const std::string_view input{value};
        std::size_t columns = 0;
        const auto parsed = std::from_chars(input.data(), input.data() + input.size(), columns);
        if (parsed.ec == std::errc{} && parsed.ptr == input.data() + input.size() &&
            columns > 0 && columns <= 10000) return columns;
    }
    return 80;
}

std::size_t visibleTerminalWidth(std::string_view text)
{
    std::size_t width = 0;
    for (const auto& cell : cells(text)) width += cell.width;
    return width;
}

std::vector<std::string> wrapTerminalText(std::string_view text, std::size_t columns,
                                        std::size_t continuationIndent)
{
    if (columns == 0) throw std::invalid_argument("Terminal width must be positive");
    const auto units = cells(text);
    // Preserve room for a wide character on even the smallest continuation line.
    const auto indent = std::min(continuationIndent, columns > 2 ? columns - 2 : 0);
    std::vector<std::string> lines;
    std::string line;
    std::size_t used = 0;
    bool hasWord = false;
    const auto flush = [&] {
        lines.push_back(std::move(line));
        line = std::string(indent, ' ');
        used = indent;
        hasWord = false;
    };
    for (std::size_t begin = 0; begin < units.size();)
    {
        if (units[begin].newline) { flush(); ++begin; continue; }
        if (units[begin].space) { ++begin; continue; }
        auto end = begin;
        std::size_t wordWidth = 0;
        while (end < units.size() && !units[end].space && !units[end].newline)
            wordWidth += units[end++].width;
        if (hasWord && used + 1 + wordWidth > columns) flush();
        if (hasWord) { line += ' '; ++used; }
        for (auto i = begin; i < end; ++i)
        {
            if (used + units[i].width > columns && used > indent) flush();
            if (units[i].width > columns - used)
            {
                // A double-width glyph cannot fit in a one-cell terminal.
                line += "\xef\xbf\xbd";
                ++used;
            }
            else { line += units[i].text; used += units[i].width; }
            hasWord = true;
        }
        begin = end;
    }
    if (hasWord || lines.empty()) lines.push_back(std::move(line));
    return lines;
}
