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


inline std::vector<int> calculatePath(World& world, std::string ownerTag, int from, int to) {
    std::cout << "try calculate\n";

    // national accessiblity
    const std::map<int, std::vector<int>>* adjacency = &countryTagFind(world.countries, ownerTag)->accessibilityGraph;

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
            std::cout << "Province " << current << " not found in adjacency\n";
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
      std::cout << "end of calculus\n";
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
// DAILY SIMULATION TICK
// ============================================================

inline void updateArmyMovement(World& world) {
    for (auto& army : world.armies) {
        if (army.path.empty()) continue;
        army.movementStage += world.armyMovementSpeed;
        std::cout << "[" << army.name << "] stage: " << army.movementStage << " path size: " << army.path.size() << "\n";
        if (army.movementStage >= 100) {
            army.movementStage -= 100;
            army.position = army.path.front();
            army.path.erase(army.path.begin());
            std::cout << "[" << army.name << "] moved to: " << army.position << "\n";
        }
    }
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

inline Army* armyPositionFind(const std::list<Army>& list, int provinceId) {
    for (auto& army : list)
        if (army.position == provinceId)
            return const_cast<Army*>(&army);
    return nullptr;
}
