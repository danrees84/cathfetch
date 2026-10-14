#ifndef LECTIONARY_LOADER_H
#define LECTIONARY_LOADER_H

#include "reading_set.h"
#include <nlohmann/json_fwd.hpp>

Lectionary parseLectionary(const nlohmann::json& document);
Lectionary loadLectionary(const std::string& filename = "data/lectionary/roman_references.json");
void validateLectionary(const Lectionary& lectionary);

#endif
