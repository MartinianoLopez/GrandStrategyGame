#pragma once

// ===============================

#include "../Model/World.hpp"
#include "../utils.hpp"
#include "Diplomacy.hpp"

// ===============================

#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>

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
    // add a flag of exiled to not engage in battles
    // take it off if the province is equal itself tag
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
// Recruitment
// ============================================================

inline void addArmy(World& world, const std::string& ownerTag, int provinceId, int power) {
    for (auto& army : world.armies) {
        if (army.position != provinceId || army.owner != ownerTag) continue;
        army.power += power;
        return;
    }

    Country* country = findCountryByTag(world.countries, ownerTag);
    if (!country) return;
    world.armies.emplace_back(provinceId, "Recruits", ownerTag, power, country->color);
}

inline bool recruitArmy(World& world, const std::string& ownerTag, int provinceId) {
    Country* country = findCountryByTag(world.countries, ownerTag);
    Province* province = findProvinceById(world.provinces, provinceId);
    // Check that the country exists.
    if (!country) return false;
    // Check that the recruitment province exists.
    if (!province) return false;

    // A country can only recruit on owned territory.
    if (province->owner != ownerTag) return false;
    // A country can only recruit on controlled territory.
    if (province->controller != ownerTag) return false;
    // Check if the country can afford the recruitment cost.
    if (country->money < world.RECRUITMENT_COST) return false;

    addArmy(world, ownerTag, provinceId, world.RECRUITMENT_POWER);
    country->money -= world.RECRUITMENT_COST;
    return true;
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

inline void removeStaleArmyPointers(World& world) {
    world.selectedArmies.erase(
        std::remove_if(
            world.selectedArmies.begin(),
            world.selectedArmies.end(),
            [&world](Army* army) {
                if (!army) return true;
                return std::find_if(
                    world.armies.begin(),
                    world.armies.end(),
                    [army](const Army& candidate) { return &candidate == army; }
                ) == world.armies.end();
            }
        ),
        world.selectedArmies.end()
    );
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

inline void joinArmies(World& world, Army& army, Army& army2){
    army2.power += army.power;
    army.power = 0;
}

inline void splitArmies(World& world){
    std::vector<Army*>& selectedArmies = world.selectedArmies;
    // only split one selected army
    if (selectedArmies.size() != 1) return;
    Army* army = selectedArmies[0];
    // don't divide the armies in the hundreds
    int split = (army->power / 2 / 1000) * 1000;
    if (split <= 0) return;
    Country* owner = findCountryByTag(world.countries, army->owner);
    if (!owner) return;
    world.armies.emplace_back(army->position, "Recruits", army->owner, split, owner->color);
    army->power -= split;
}

inline void scanForEnemies(World& world, Army& army) {
    for (Army* other : findArmiesOnProvinceId(world.armies, army.position)) {
        if (other == &army) continue;
        if (isAtWar(world, army.owner, other->owner)) fight(army, *other);
    }
}

inline void scanForAliesAndRegroup(World& world, Army& army) {
    std::vector<Army*> armiesInProvince = findArmiesOnProvinceId(world.armies, army.position);
    for (Army* other : armiesInProvince) {
        if (other == &army) continue;
        if(other->owner == army.owner){
            joinArmies(world, army, *other);
        }
            
    }
    
}

// ============================================================
// Invasion
// ============================================================

inline void reloadControlTex(World& world){
    world.controlTex = surfaceToTexture(world.renderer, world.controlSur);
}

inline void occupyProvince(World& world, Province* province, Country* country) {
    province->controller = country->tag;
    uint32_t color = SDL_MapRGB(world.controlSur->format, country->color.r, country->color.g, country->color.b);
    Uint8* pixels = (Uint8*)world.controlSur->pixels;
    int pitch = world.controlSur->pitch;
    int bpp = world.controlSur->format->BytesPerPixel;

    SDL_LockSurface(world.controlSur);
    for (const auto& [x, y] : province->shape)
        *(Uint32*)(pixels + y * pitch + x * bpp) = color;
    SDL_UnlockSurface(world.controlSur);

    reloadControlTex(world);
}

inline void tryOccupyProvince(World& world, Army& army) {
    Country* country = findCountryByTag(world.countries, army.owner);
    Province* province = findProvinceById(world.provinces, army.position);

    if (!country || !province) return;

    // if the province is an enemy province
    if (isEnemyProvinceForCountry(world, *country, *province)){
        occupyProvince(world, province, country);
    }

    // if the province is an own province controlled by an enemy in a war
    if (province->owner == country->tag){
        occupyProvince(world, province, country);
    }
        
}

// ============================================================
// ARMY MOVEMENT SYSTEM
// ============================================================

inline void moveArmy(World& world, Army& army) {
    if (army.path.empty()) return;

    army.position = army.path.front();
    army.path.erase(army.path.begin());

    scanForEnemies(world,army);
    tryOccupyProvince(world, army);
    scanForAliesAndRegroup(world, army);
}

inline void createArmyMovement(World& world,Army* army, int from, int to) {
    std::vector<int> path = calculatePath(world, army->owner, from, to);
    if (path.empty()) return;    
       army -> path = path; 
    return;
}

inline void updateArmyMovement(World& world) {

    for (auto& army : world.armies) {
        // if army is in final position continue with the next
        if (army.path.empty()) continue;
        // add movement progress
        army.movementStage += world.armyMovementSpeed;
        // if movement progress not full continue
        if (army.movementStage < 100) continue;
            // remove the progress
            army.movementStage -= 100;
            // move the army
            moveArmy(world, army);
    }
    // remove armies with 0 troops
    remove0Armies(world.armies);
    removeStaleArmyPointers(world);
}