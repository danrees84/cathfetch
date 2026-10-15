// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Daniel Rees
// See LICENSE for licence terms and the disclaimer of warranty.

#ifndef LECTIONARY_LOADER_H
#define LECTIONARY_LOADER_H

#include "reading_set.h"
#include <nlohmann/json_fwd.hpp>

Lectionary parseLectionary(const nlohmann::json& document);
Lectionary loadLectionary(const std::string& filename = "");
void validateLectionary(const Lectionary& lectionary);

#endif
