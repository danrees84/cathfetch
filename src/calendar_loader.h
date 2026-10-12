#ifndef CALENDAR_LOADER_H
#define CALENDAR_LOADER_H

#include <string>
#include <vector>
#include "fixed_celebration.h"

std::vector<FixedCelebration> loadFixedCelebrations(
    const std::string& filename);

#endif
