// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#include "data_path.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

std::filesystem::path dataFile(std::string_view relativePath)
{
    namespace fs = std::filesystem;
    const fs::path relative{relativePath};
    if (relative.is_absolute()) throw std::invalid_argument("Bundled data path must be relative");
    for (const auto& component : relative)
        if (component == "..") throw std::invalid_argument("Bundled data path cannot leave its directory");

    if (const char* configured = std::getenv("CATHFETCH_DATA_DIR"); configured && *configured)
    {
        const auto file = fs::path{configured} / relative;
        if (fs::is_regular_file(file)) return file;
        // An explicit override must not silently fall back to a different edition.
        throw std::runtime_error("Data file not found in CATHFETCH_DATA_DIR: " + file.string());
    }

    std::error_code error;
    const auto executable = fs::read_symlink("/proc/self/exe", error);
    if (error) throw std::runtime_error("Cannot locate Cathfetch executable; set CATHFETCH_DATA_DIR");
    const auto directory = executable.parent_path();
    const auto installed = directory.parent_path() / "share/cathfetch";
    const auto development = directory.parent_path() / "data";
    // Installed layout: PREFIX/bin and PREFIX/share/cathfetch. Development
    // layout: PROJECT/build and PROJECT/data, with the generated Bible in build.
    const bool installedLayout = fs::is_directory(installed);
    auto file = (installedLayout ? installed : development) / relative;
    if (!installedLayout && relative == "bibles/en_dra.json")
        file = directory / "en_dra.json";
    if (fs::is_regular_file(file)) return file;
    throw std::runtime_error("Cathfetch data file not found: " + file.string() +
                             "; set CATHFETCH_DATA_DIR to the data directory");
}
