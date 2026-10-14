#include "terminal_display.h"
#include "liturgical_format.h"
#include "terminal_format.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace
{
const std::array<std::string, 8> cross{
    "          ███", "          ███", "      ███████████", "          ███",
    "          ███", "          ███", "          ███", "          ███"
};

std::string colourDot(LiturgicalColour colour)
{
    return terminalColourCode(colour) + "●\033[0m ";
}

std::string massLabel(std::string_view mass)
{
    if (mass == "passion") return "Passion liturgy";
    if (mass == "evening") return "Evening Mass";
    if (mass == "night") return "Night Mass";
    if (mass == "dawn") return "Dawn Mass";
    if (mass == "vigil") return "Vigil Mass";
    if (mass == "vigil-extended") return "Extended Vigil Mass";
    return "Day Mass";
}

std::vector<std::string> informationRows(const LiturgicalDay& day, std::size_t columns,
                                        const LectionaryDay* readings)
{
    auto rows = wrapTerminalText(formatCalendarDate(day.date), columns);
    const auto seasonColour = seasonalColour(day.season);
    const auto append = [&](std::string text, bool hasDot) {
        auto wrapped = wrapTerminalText(text, columns, hasDot ? 2 : 0);
        rows.insert(rows.end(), wrapped.begin(), wrapped.end());
    };
    append((seasonColour ? colourDot(*seasonColour) : "") + liturgicalPeriodName(day.season),
           seasonColour.has_value());
    const bool hasDot = day.colour && (day.observanceId != "temporal" || day.colour != seasonColour);
    std::string observance = (hasDot ? colourDot(*day.colour) : "") + day.observanceName;
    if (day.rank) observance += " · " + std::string{celebrationRankName(day.rank)};
    if (day.provisionalTransfer) observance += " (provisional transfer)";
    append(std::move(observance), hasDot);
    if (readings)
    {
        rows.emplace_back();
        if (readings->status == ReadingStatus::NoDaytimeMass)
            append("No daytime Mass · Easter Vigil available with --mass=vigil", false);
        else if (readings->status != ReadingStatus::Available || !readings->selected)
            append("Readings · assignment not yet verified for this day or Mass form", false);
        else
        {
            append("Readings · Sunday " + std::string{cycleName(readings->cycles.sunday)} +
                " / Weekday " + std::string{cycleName(readings->cycles.weekday)} + " · " +
                massLabel(readings->selected->mass), false);
            if (readings->selected->verification == ReferenceVerification::PublishedTable)
                append("Published reference tables · 2024 edition collation pending", false);
            if (readings->selected->use == ReadingUse::Accommodated)
                append("Suggested selection · other permitted readings may be chosen", false);
            for (const auto& reading : readings->selected->readings)
            {
                std::string label = reading.role == "first" ? "First" : reading.role == "psalm" ? "Psalm" :
                    reading.role == "second" ? "Second" : reading.role == "gospel" ? "Gospel" :
                    reading.role == "acclamation" ? "Acclamation" : reading.role == "epistle" ? "Epistle" :
                    reading.role == "procession_gospel" ? "Procession Gospel" : reading.role;
                if (label.starts_with("reading_")) label = "Reading " + label.substr(8);
                if (label.starts_with("psalm_")) label = "Psalm " + label.substr(6);
                append(label + ": " + reading.reference, false);
                for (const auto& option : reading.alternatives)
                    append("  or " + option.reference + " (" + option.condition + ")", false);
            }
            if (!readings->alternatives.empty())
                append("Other reading sets or Mass forms: " + std::to_string(readings->alternatives.size()), false);
        }
    }
    return rows;
}
} // namespace

std::string renderLiturgicalDay(const LiturgicalDay& day, bool showCross, std::size_t columns,
                               const LectionaryDay* readings)
{
    if (columns == 0) throw std::invalid_argument("Terminal width must be positive");
    if (!day.unresolvedReason.empty())
    {
        std::string diagnostic = formatCalendarDate(day.date) + ": " + day.unresolvedReason;
        for (const auto& conflict : day.conflictingObservances) diagnostic += "; " + conflict.name;
        throw std::logic_error(diagnostic);
    }
    constexpr std::size_t artworkWidth = 17;
    constexpr std::size_t gap = 6;
    constexpr std::size_t textColumn = artworkWidth + gap;
    constexpr std::size_t minimumTextWidth = 32;
    const bool sideBySide = showCross && columns >= textColumn + minimumTextWidth;
    const auto rows = informationRows(day, sideBySide ? columns - textColumn : columns, readings);
    std::string output;
    if (sideBySide)
    {
        // Text starts on the second artwork row, as in the original design.
        for (std::size_t row = 0; row < std::max(cross.size(), rows.size() + 1); ++row)
        {
            const auto artwork = row < cross.size() ? cross[row] : "";
            output += artwork;
            if (row > 0 && row <= rows.size())
                output += std::string(textColumn - visibleTerminalWidth(artwork), ' ') + rows[row - 1];
            output += '\n';
        }
    }
    else
    {
        if (showCross)
        {
            if (columns >= 11)
            {
                const std::string padding((columns - 11) / 2, ' ');
                for (const auto& row : cross) output += padding + row.substr(6) + '\n';
            }
            else
            {
                // The full eleven-cell arm is physically impossible below 11.
                const auto arm = columns >= 3 ? "███" : "█";
                const auto stem = columns >= 3 ? " █" : "█";
                output += std::string{stem} + '\n' + arm + '\n' + stem + '\n';
            }
            output += '\n';
        }
        for (const auto& row : rows) output += row + '\n';
    }
    return output;
}

void printLiturgicalDay(const LiturgicalDay& day, bool showCross, const LectionaryDay* readings)
{
    std::cout << renderLiturgicalDay(day, showCross, terminalColumns(), readings);
}
