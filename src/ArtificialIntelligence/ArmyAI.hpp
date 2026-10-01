#pragma once

#include "../Simulation/Army.hpp"
#include "../Simulation/Diplomacy.hpp"

#include <algorithm>
#include <limits>

// ============================================================
// RECLUTAMIENTO
// ============================================================

inline bool hasActiveWarForCountry(const World& world, const std::string& tag) {
    for (const auto& war : world.activeWars) {
        if (std::find(war.attackerCountries.begin(), war.attackerCountries.end(), tag) != war.attackerCountries.end()) return true;
        if (std::find(war.defenderCountries.begin(), war.defenderCountries.end(), tag) != war.defenderCountries.end()) return true;
    }
    return false;
}

inline void expandMilitary(World& world, Country& country) {
    if (country.money <= world.RECRUITMENT_COST) return;
    //find a province to recruit
    for (const auto& province : world.provinces) {
        if (province.owner != country.tag) continue;
		if (province.controller != country.tag) continue;

        recruitArmy(world, country.tag, province.id);
        return;
    }
}

// ============================================================
// INVASION
// ============================================================

inline bool isEnemyProvinceForCountry(const World& world, const Country& country, const Province& province) {
    if (province.owner == country.tag && province.controller == country.tag) return false;

    const bool ownerIsEnemy = province.controller != "NONE" && province.controller != country.tag && isAtWar(world, country.tag, province.owner);
    const bool controllerIsEnemy = !province.controller.empty() && province.controller != country.tag && isAtWar(world, country.tag, province.controller);

    return ownerIsEnemy || controllerIsEnemy;
}

inline int findClosestEnemyProvinceForArmy(World& world, const Country& country, const Army& army) {
    int bestProvinceId = -1;
    int bestDistance = std::numeric_limits<int>::max();

    for (const auto& province : world.provinces) {
        if (!isEnemyProvinceForCountry(world, country, province)) continue;

        std::vector<int> path = calculatePath(world, country.tag, army.position, province.id);
        if (path.empty()) continue;

        const int distance = static_cast<int>(path.size());
        if (distance < bestDistance) {
            bestDistance = distance;
            bestProvinceId = province.id;
        }
    }

    return bestProvinceId;
}

inline void orderArmyToAttack(World& world, Country& country, Army& army) {
    if (!army.path.empty() || army.power <= 0) return;

    const int targetProvinceId = findClosestEnemyProvinceForArmy(world, country, army);
    if (targetProvinceId < 0) return;

    createArmyMovement(world, &army, army.position, targetProvinceId);
}

inline void invadeEnemies(World& world, Country& country) {
    if (!hasActiveWarForCountry(world, country.tag)) return;

    for (auto& army : world.armies) {
        if (army.owner != country.tag) continue;
        if (army.path.empty()) orderArmyToAttack(world, country, army);
    }
}

inline void executeMilitaryStrategyForCountryAI(World& world, Country& country) {
    expandMilitary(world, country);
    invadeEnemies(world, country);
}

inline void executeMilitaryStrategyAI(World& world) {
    for (auto& country : world.countries) {
        if (country.tag == world.playerCountry) continue;
        executeMilitaryStrategyForCountryAI(world, country);
    }
}
