
// ================================

#include "../Model/World.hpp"

// ================================

// ================================
inline void collectTaxes(World& world) {
    std::unordered_map<std::string, int> taxes;

    // calculate taxes
    for (auto& province : world.provinces)
        if (!province.controller.empty()) taxes[province.controller] += province.localEconomy;
        
    // asign revenew
    for (auto& country : world.countries)
        country.money += taxes[country.tag];
}