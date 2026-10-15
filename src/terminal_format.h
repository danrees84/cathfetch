#ifndef TERMINAL_FORMAT_H
#define TERMINAL_FORMAT_H

#include <string>
#include <string_view>
#include <vector>
#include <cstddef>
#include "liturgical_colour.h"

std::string terminalColourCode(LiturgicalColour colour);

// Width of stdout's terminal, then a valid COLUMNS value, then 80 cells.
std::size_t terminalColumns();
// Display cells are not byte counts: colours occupy no cells, and some Unicode
// characters occupy two. Use these helpers instead of string::size() for layout.
std::size_t visibleTerminalWidth(std::string_view text);
std::string stripTerminalEscapes(std::string_view text);
std::vector<std::string> wrapTerminalText(std::string_view text, std::size_t columns,
                                        std::size_t continuationIndent = 0);

#endif
