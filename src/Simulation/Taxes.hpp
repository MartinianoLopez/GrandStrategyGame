#include "../Model/World.hpp"

inline void collectTaxes(World& world) {
    std::unordered_map<std::string, int> taxes;

    for (auto& province : world.provinces)
        if (!province.owner.empty()) taxes[province.owner] += 1; // replace with the develompent of the province

    for (auto& country : world.countries)
        country.money += taxes[country.tag];
}