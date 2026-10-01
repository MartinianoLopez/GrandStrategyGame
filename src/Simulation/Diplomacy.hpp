#pragma once

// ======================================

#include "../Model/World.hpp"
#include "../utils.hpp"
#include "../Model/DataProcessing.hpp"
#include "../View/LabelBuilder.hpp"

// ======================================

#include <string>

// ======================================
inline bool isAtWar(const World& world, const std::string& a, const std::string& b){
    for (const auto& w : world.activeWars) {
        auto in = [](const std::vector<std::string>& v, const std::string& t){ return std::find(v.begin(), v.end(), t) != v.end(); };
        if ((in(w.attackerCountries, a) && in(w.defenderCountries, b)) || (in(w.defenderCountries, a) && in(w.attackerCountries, b))) return true;
    }
    return false;
}
inline bool isEnemyProvinceForCountry(const World& world, const Country& country, const Province& province) {
    // if the country is at war with the controller of that province
    return isAtWar(world, country.tag, province.controller);
}

inline std::vector<std::string> getAllies(const Country& c){
    std::vector<std::string> allies;
    for (const auto& r : c.relationships) if (r.stateOfRelations == stateOfRelation::ALLIANCE) allies.push_back(r.tag);
    return allies;
}

inline Relationship* findRelationship(std::vector<Relationship>& relationships, const std::string& tag){
    auto it = std::find_if(relationships.begin(), relationships.end(), [&](const Relationship& relationship){
        return relationship.tag == tag;
    });
    return it == relationships.end() ? nullptr : &*it;
}

inline const Relationship* findRelationship(const std::vector<Relationship>& relationships, const std::string& tag){
    auto it = std::find_if(relationships.begin(), relationships.end(), [&](const Relationship& relationship){
        return relationship.tag == tag;
    });
    return it == relationships.end() ? nullptr : &*it;
}

inline bool isAllied(const World& world, const std::string& a, const std::string& b){
    if (a == "NONE" || b == "NONE" || a == b) return false;

    const Country* countryA = findCountryByTag(world.countries, a);
    const Country* countryB = findCountryByTag(world.countries, b);
    if (!countryA || !countryB) return false;

    const Relationship* relA = findRelationship(countryA->relationships, b);
    const Relationship* relB = findRelationship(countryB->relationships, a);
    return (relA && relA->stateOfRelations == stateOfRelation::ALLIANCE) ||
           (relB && relB->stateOfRelations == stateOfRelation::ALLIANCE);
}

inline bool canOfferAlliance(const World& world, const std::string& proposer, const std::string& target){
    if (proposer == "NONE" || target == "NONE" || proposer == target || isAtWar(world, proposer, target)) return false;
    Country* proposerCountry = findCountryByTag(world.countries, proposer);
    Country* targetCountry = findCountryByTag(world.countries, target);
    if (!proposerCountry || !targetCountry) return false;

    for (const auto& relationship : proposerCountry->relationships)
        if (relationship.tag == target && relationship.stateOfRelations == stateOfRelation::ALLIANCE) return false;

    const Relationship* targetRelationship = findRelationship(targetCountry->relationships, proposer);
    if (!targetRelationship) return true;

    return targetRelationship->opinion > world.ALLIANCE_ACCEPTANCE_RELATION_THRESHOLD;
}

inline void offerAlliance(World& world, const std::string& proposer, const std::string& target){
    if (!canOfferAlliance(world, proposer, target)) return;

    Country* proposerCountry = findCountryByTag(world.countries, proposer);
    Country* targetCountry = findCountryByTag(world.countries, target);
    auto setAlliance = [](Country& country, const std::string& otherTag){
        for (auto& relationship : country.relationships) {
            if (relationship.tag == otherTag) {
                relationship.stateOfRelations = stateOfRelation::ALLIANCE;
                return;
            }
        }
        country.addRelationship(Relationship(otherTag, stateOfRelation::ALLIANCE));
    };

    setAlliance(*proposerCountry, target);
    setAlliance(*targetCountry, proposer);
    SDL_DestroyTexture(world.activeDiplomaticMap);
    world.activeDiplomaticMap = nullptr;
}

inline void breakAlliance(World& world, const std::string& a, const std::string& b){
    if (a == "NONE" || b == "NONE" || a == b) return;

    Country* countryA = findCountryByTag(world.countries, a);
    Country* countryB = findCountryByTag(world.countries, b);
    if (!countryA || !countryB) return;

    auto removeAlliance = [](Country& country, const std::string& otherTag){
        Relationship* relationship = findRelationship(country.relationships, otherTag);
        if (!relationship) return;
        relationship->stateOfRelations = stateOfRelation::PEACE;
    };

    removeAlliance(*countryA, b);
    removeAlliance(*countryB, a);
    SDL_DestroyTexture(world.activeDiplomaticMap);
    world.activeDiplomaticMap = nullptr;
}

inline std::vector<activeWar> getWarsOf(const World& world, const std::string& tag){
    std::vector<activeWar> wars;
    for (const auto& w : world.activeWars)
        if (std::find(w.attackerCountries.begin(), w.attackerCountries.end(), tag) != w.attackerCountries.end() ||
            std::find(w.defenderCountries.begin(), w.defenderCountries.end(), tag) != w.defenderCountries.end())
            wars.push_back(w);
    return wars;
}

inline bool canDemandProvinceInPeaceTreaty(const World& world, const std::string& player,
                                            const std::string& target, int provinceId){
    if (player == "NONE" || target == "NONE" || player == target) return false;
    const Province* province = findProvinceById(world.provinces, provinceId);
    if (!province) return false;

    for (const auto& war : world.activeWars) {
        const auto contains = [](const std::vector<std::string>& countries, const std::string& tag){
            return std::find(countries.begin(), countries.end(), tag) != countries.end();
        };
        const std::vector<std::string>* playerSide = nullptr;
        const std::vector<std::string>* targetSide = nullptr;
        if (contains(war.attackerCountries, player) && contains(war.defenderCountries, target)) {
            playerSide = &war.attackerCountries;
            targetSide = &war.defenderCountries;
        } else if (contains(war.defenderCountries, player) && contains(war.attackerCountries, target)) {
            playerSide = &war.defenderCountries;
            targetSide = &war.attackerCountries;
        }
        if (!playerSide || !contains(*targetSide, province->owner)) continue;
        return contains(*playerSide, province->controller);
    }
    return false;
}

inline bool canOfferProvinceInPeaceTreaty(const World& world, const std::string& player,
                                           int provinceId){
    const Province* province = findProvinceById(world.provinces, provinceId);
    return province && province->terrainType != TerrainType::OCEAN && province->owner == player;
}

inline bool offerPeaceTreaty(World& world, const std::string& player, const std::string& target,
                             const std::vector<int>& demandedProvinces,
                             const std::vector<int>& offeredProvinces){
    if (demandedProvinces.empty() && offeredProvinces.empty()) return false;
    for (int provinceId : demandedProvinces)
        if (!canDemandProvinceInPeaceTreaty(world, player, target, provinceId)) return false;
    for (int provinceId : offeredProvinces)
        if (!canOfferProvinceInPeaceTreaty(world, player, provinceId)) return false;

    auto warIt = std::find_if(world.activeWars.begin(), world.activeWars.end(), [&](const activeWar& war){
        const auto contains = [](const std::vector<std::string>& countries, const std::string& tag){
            return std::find(countries.begin(), countries.end(), tag) != countries.end();
        };
        return (contains(war.attackerCountries, player) && contains(war.defenderCountries, target)) ||
               (contains(war.defenderCountries, player) && contains(war.attackerCountries, target));
    });
    if (warIt == world.activeWars.end()) return false;

    const std::vector<std::string> attackerCountries = warIt->attackerCountries;
    const std::vector<std::string> defenderCountries = warIt->defenderCountries;
    for (int provinceId : demandedProvinces) {
        Province* province = findProvinceById(world.provinces, provinceId);
        province->owner = player;
        province->controller = player;
    }
    for (int provinceId : offeredProvinces) {
        Province* province = findProvinceById(world.provinces, provinceId);
        province->owner = target;
        province->controller = target;
    }

    std::unordered_set<std::string> warParticipants(attackerCountries.begin(), attackerCountries.end());
    warParticipants.insert(defenderCountries.begin(), defenderCountries.end());
    if (world.controlSur && world.controlSur->format) SDL_LockSurface(world.controlSur);
    for (auto& province : world.provinces) {
        if (!warParticipants.count(province.owner) || province.controller == province.owner) continue;
        if (!warParticipants.count(province.controller)) continue;

        province.controller = province.owner;
        Uint32 controlColor = SDL_MapRGBA(world.controlSur->format, 0, 0, 0, 0);
        if (Country* owner = findCountryByTag(world.countries, province.owner)) {
            controlColor = SDL_MapRGB(world.controlSur->format, owner->color.r, owner->color.g, owner->color.b);
        }
        for (const auto& [x, y] : province.shape) {
            Uint8* pixel = static_cast<Uint8*>(world.controlSur->pixels)
                         + y * world.controlSur->pitch
                         + x * world.controlSur->format->BytesPerPixel;
            *reinterpret_cast<Uint32*>(pixel) = controlColor;
        }
    }
    if (world.controlSur && world.controlSur->format) SDL_UnlockSurface(world.controlSur);

    if (world.controlSur) {
        SDL_Texture* updatedControlTexture = surfaceToTexture(world.renderer, world.controlSur);
        if (updatedControlTexture) {
            SDL_DestroyTexture(world.controlTex);
            world.controlTex = updatedControlTexture;
        }
    }
    world.activeWars.erase(warIt);

    SDL_Surface* previousCountryLayer = world.countriesImg;
    SDL_Texture* previousCountryTexture = world.countriesTex;
    buildCountriesLayer(world);
    if (world.countriesImg != previousCountryLayer) {
        SDL_Texture* updatedCountryTexture = surfaceToTexture(world.renderer, world.countriesImg);
        if (updatedCountryTexture) {
            world.countriesTex = updatedCountryTexture;
            SDL_DestroyTexture(previousCountryTexture);
            SDL_FreeSurface(previousCountryLayer);
        } else {
            SDL_FreeSurface(world.countriesImg);
            world.countriesImg = previousCountryLayer;
        }
    }
    buildCountryLabels(world);
    findFrontiersBetweenCountries(world);
    generateFrontierStyle(world, "country_frontiers_thin", world.countryFrontiers, 1.5f, {0, 0, 0, 255});
    generateFrontierStyle(world, "country_frontiers_thick", world.countryFrontiers, 2.5f, {0, 0, 0, 255});

    SDL_DestroyTexture(world.activeAccessibilityMap);
    world.activeAccessibilityMap = nullptr;
    SDL_DestroyTexture(world.activeDiplomaticMap);
    world.activeDiplomaticMap = nullptr;
    world.countryoftheAccesibilityMap.clear();

    for (const auto& attacker : attackerCountries) {
        for (const auto& defender : defenderCountries) {
            if (isAtWar(world, attacker, defender)) continue;
            for (const auto& [countryTag, otherTag] : {std::pair<std::string, std::string>{attacker, defender}, {defender, attacker}}) {
                Country* country = findCountryByTag(world.countries, countryTag);
                if (!country) continue;
                auto& accessible = country->accessibleCountries;
                accessible.erase(std::remove(accessible.begin(), accessible.end(), otherTag), accessible.end());
                reloadAccesibilityGraph(world, country);
            }
        }
    }

    return true;
}

inline void declareWar(World& world, const std::string& attacker, const std::string& defender){
    if (defender == "NONE" || defender == attacker) return;
    Country* a = findCountryByTag(world.countries, attacker);
    Country* d = findCountryByTag(world.countries, defender);
    if (!a || !d) return;
    // get the allies of each country
    auto aAllies = getAllies(*a);
    auto dAllies = getAllies(*d);

    auto has = [](const std::vector<std::string>& v, const std::string& t){ return std::find(v.begin(), v.end(), t) != v.end(); };
    // create active war
    activeWar war; war.mainAttacker = attacker; war.mainDefender = defender; war.warProgressForAttackers = 0;
    war.attackerCountries = {attacker}; war.defenderCountries = {defender};
    // add allies to the war
    for (auto& al : aAllies) if (al != defender && !has(dAllies, al)) war.attackerCountries.push_back(al);
    for (auto& al : dAllies) if (al != attacker && !has(aAllies, al)) war.defenderCountries.push_back(al);
    // save war
    world.activeWars.push_back(war);
    SDL_DestroyTexture(world.activeDiplomaticMap);
    world.activeDiplomaticMap = nullptr;

    // add access between contries
    for (auto& x : war.attackerCountries) for (auto& y : war.defenderCountries) {
        if (Country* cx = findCountryByTag(world.countries, x)) cx->addAccesibleCountries(y);
        if (Country* cy = findCountryByTag(world.countries, y)) cy->addAccesibleCountries(x);
    }
    // add the acceess to the graphs
    for (auto* side : {&war.attackerCountries, &war.defenderCountries})
        for (auto& t : *side) if (Country* c = findCountryByTag(world.countries, t)) reloadAccesibilityGraph(world, c);
}