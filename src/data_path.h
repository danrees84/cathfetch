// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#ifndef DATA_PATH_H
#define DATA_PATH_H

#include <filesystem>
#include <string_view>

// Find bundled application data without depending on the caller's directory.
// CATHFETCH_DATA_DIR, when set, names a complete replacement data directory.
std::filesystem::path dataFile(std::string_view relativePath);

#endif
