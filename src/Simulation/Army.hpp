#pragma once
#include "../Model/World.hpp"
#include "../utils.hpp"
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <iostream>

// ============================================================
// PATH PLANNING
// ============================================================
inline Army* FindArmyOnProvinceId(const std::list<Army>& list, int provinceId) {
    for (auto& army : list)
        if (army.position == provinceId)
            return const_cast<Army*>(&army);
    return nullptr;
}
inline std::vector<Army*> findArmiesOnProvinceId(std::list<Army>& armies, int provinceId) {
    std::vector<Army*> result;
    for (auto& army : armies)
        if (army.position == provinceId)
            result.push_back(&army);
    return result;
}

inline std::vector<int> calculatePath(World& world, std::string ownerTag, int from, int to) {
    //std::cout << "try calculate\n";

    // national accessiblity
    const std::map<int, std::vector<int>>* adjacency = &findCountryByTag(world.countries, ownerTag)->accessibilityGraph;

    // exiled armies
    //if (army.exiled == true){
    //    adjacency = &world.adjacencyGraph; 
    //}
    
    std::unordered_map<int, int> parent;
    std::queue<int> queue;
    queue.push(from);
    parent[from] = -1;

    while (!queue.empty()) {
        int current = queue.front(); queue.pop();
        if (current == to) break;

        auto it = adjacency->find(current);
        if (it == adjacency->end()) {
            // std::cout << "Province " << current << " not found in adjacency\n";
            continue;
        }

        for (int neighbor : it->second) {
            if (parent.find(neighbor) == parent.end()) {
                parent[neighbor] = current;
                queue.push(neighbor);
            }
        }
    }

    if (parent.find(to) == parent.end()) return {};

    std::vector<int> path;
    for (int p = to; p != -1; p = parent[p])
        path.push_back(p);
    std::reverse(path.begin(), path.end());
    path.erase(path.begin());
      // std::cout << "end of calculus\n";
    return path;
}

// ============================================================
// ARMY MOVEMENT SYSTEM
// ============================================================


inline void createArmyMovement(World& world,Army* army, int from, int to) {
    std::vector<int> path = calculatePath(world, army->owner, from, to);
    if (path.empty()) return;    
       army -> path = path; 
    return;
}
// ============================================================
// Recruitment
// ============================================================
inline void recruitArmy(World& world) {
    Country* country = findCountryByTag(world.countries, world.playerCountry);
    if (!country) { std::cerr << "ERROR: country not found!\n"; return; }
    world.armies.emplace_back(world.objectiveProvince, "Recruits", world.playerCountry, 1000, country->color);
}

inline void tryToRecruitArmy(World& world) {
    Country* country = findCountryByTag(world.countries, world.playerCountry);
    if (country->money >= 100) {
        country->money -= 100;
        recruitArmy(world);
    }
    world.recruitOneUnit = false;
}

// ============================================================
// Battles
// ============================================================
inline void removeArmy(std::list<Army>& armies, Army& army) {
    for (auto it = armies.begin(); it != armies.end(); ++it) {
        if (&(*it) == &army) {
            armies.erase(it);
            return;
        }
    }
}
inline void remove0Armies(std::list<Army>& armies) {
    for (auto it = armies.begin(); it != armies.end();) {
        if (it->power == 0) it = armies.erase(it);
        else ++it;
    }
}

inline void fight(Army& a, Army& b) {
    int aTroops = a.power - b.power;
    int bTroops = b.power - a.power;
    a.power = std::max(0, aTroops);
    b.power = std::max(0, bTroops);
}

inline bool isAtWar(std::vector<Relationship>& warRelations, const std::string& owner) {
    for (Relationship& r : warRelations)
        if (r.tag == owner) return true;
    return false;
}

inline void scanForEnemies(World& world, Army& army) {
    std::vector<Army*> armiesInProvince = findArmiesOnProvinceId(world.armies, army.position);
    Country* country = findCountryByTag(world.countries, army.owner);
    if (!country) return;
    std::vector<Relationship> warRelations = country->getWarRelations();

    for (Army* other : armiesInProvince) {
        if (other == &army) continue;
        if (isAtWar(warRelations, other->owner))
            fight(army, *other);
    }
    
}
inline void occupyProvince(World& world, Army& army) {
    Country* country = findCountryByTag(world.countries, army.owner);
    Province* province = provinceFindById(world.provinces, army.position);
    std::vector<Relationship> warRelations = country->getWarRelations();
    if (isAtWar(warRelations, province->owner)){
        province->controller = army.owner;
    }
}

inline void updateArmyMovement(World& world) {
    for (auto& army : world.armies) {
        if (army.path.empty()) continue;
        army.movementStage += world.armyMovementSpeed;
        if (army.movementStage >= 100) {
            army.movementStage -= 100;
            army.position = army.path.front();
            army.path.erase(army.path.begin());
            scanForEnemies(world,army);
            occupyProvince(world, army);
            // std::cout << "[" << army.name << "] moved to: " << army.position << "\n";
        }
    }
    remove0Armies(world.armies);
}



/*
// ============================================================
// COLLISION & COMBAT RESOLUTION
// ============================================================

checkForCollisions(army){
    for each otherArmy at same position:
        if enemy and atWar:
            checkForBattles(army, otherArmy)
    if province.controller != army.owner and atWar:
        startSiege(army, province)
}

checkForBattles(army, otherArmy){
    if province.hasBattle:
        joinBattle(army)
    else:
        startBattle(army, otherArmy)
}

startBattle(army, otherArmy){
    // automatic battle for now
    army.troops.multiplied by random from 1 to 2 >= otherarmy.troops multiplied by random from 1 to 2
    winner is bigger
    calculate engagement mortality for loser()
    loser army.troops -> loser.troops - loser.troops*mortality
    calculate engagement mortality for winner()
    winer army.troops -> winner.troops - winner.troops*mortality
}
    
startSiege(army, province):
    // automatic siege for now
    province.controller = army.owner
    redrawCountriesImg()
*/

inline void moveArmy(Army& army, int toProvinceId) {
    army.position = toProvinceId;
}

