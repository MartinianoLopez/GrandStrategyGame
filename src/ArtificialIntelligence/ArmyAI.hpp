#pragma once

#include "../Simulation/Army.hpp"
#include "../Simulation/Diplomacy.hpp"

#include <algorithm>
#include <random>

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

inline int findNearbyEnemyProvinceForArmy(World& world, const Country& country, const Army& army) {
    const Province* currentProvince = findProvinceById(world.provinces, army.position);
    if (!currentProvince) return -1;

    constexpr std::size_t candidateLimit = 5;
    std::vector<std::pair<long long, int>> candidates;

    for (const auto& province : world.provinces) {
        if (!isEnemyProvinceForCountry(world, country, province)) continue;

        const long long dx = province.center.x - currentProvince->center.x;
        const long long dy = province.center.y - currentProvince->center.y;
        const long long distanceSquared = dx * dx + dy * dy;

        auto insertionPoint = std::lower_bound(
            candidates.begin(), candidates.end(), distanceSquared,
            [](const auto& candidate, long long distance) { return candidate.first < distance; });
        if (candidates.size() == candidateLimit && insertionPoint == candidates.end()) continue;

        candidates.insert(insertionPoint, {distanceSquared, province.id});
        if (candidates.size() > candidateLimit) candidates.pop_back();
    }

    if (candidates.empty()) return -1;

    static std::mt19937 randomEngine(std::random_device{}());
    std::uniform_int_distribution<std::size_t> chooseCandidate(0, candidates.size() - 1);
    return candidates[chooseCandidate(randomEngine)].second;
}

inline void orderArmyToAttack(World& world, Country& country, Army& army) {
    if (!army.path.empty() || army.power <= 0) return;

    const int targetProvinceId = findNearbyEnemyProvinceForArmy(world, country, army);
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
