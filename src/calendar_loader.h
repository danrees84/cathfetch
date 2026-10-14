#ifndef CALENDAR_LOADER_H
#define CALENDAR_LOADER_H

#include <string>
#include <vector>
#include <nlohmann/json_fwd.hpp>
#include "fixed_celebration.h"

std::vector<FixedCelebration> loadFixedCelebrations(const std::string& filename);
FixedCelebration parseFixedCelebration(const nlohmann::json& entry,
                                      const std::string& sourceCalendar);
void updateCelebrationMetadata(FixedCelebration& celebration,
                               const nlohmann::json& changes,
                               const std::string& sourceCalendar);
void validateCalendar(const std::vector<FixedCelebration>& celebrations);

#endif
