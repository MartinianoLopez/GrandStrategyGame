#pragma once

#include "../Model/World.hpp"

#include <algorithm>

inline bool areUiConditionsMet(World& world, const UIElement& element) {
    return std::all_of(
        element.visibleWhen.begin(),
        element.visibleWhen.end(),
        [&world](const std::string& conditionName) {
            auto condition = world.ui.conditions.find(conditionName);
            return condition != world.ui.conditions.end() && condition->second(world);
        }
    );
}
