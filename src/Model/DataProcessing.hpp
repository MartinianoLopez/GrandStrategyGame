#pragma once

//===========================

#include "World.hpp"
#include "../utils.hpp"

//===========================

#include <cstdint>
#include <string>
#include <SDL2/SDL_image.h>
#include <unordered_set>
#include <SDL2/SDL_image.h>

//============================

inline void findFrontiers(World& world) {   
    SDL_Surface* img = world.provincesBmp;
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> frontierList;
    int imgW = img->w;
    int imgH = img->h;

    for (int y = 0; y < imgH; y++){
        for (int x = 0; x < imgW; x++) {
            uint32_t current = getPixelColor(img, x, y);
            if (x + 1 < imgW) { 
                uint32_t next = getPixelColor(img, x + 1, y);
                if (next != current) frontierList[{current, next}].push_back({x + 0.5f, (float)y});
            }
        }
    }

    for (int y = 0; y < imgH; y++){ 
        for (int x = 0; x < imgW; x++) {  
            uint32_t current = getPixelColor(img, x, y);
            if (y + 1 < imgH) {  
                uint32_t next = getPixelColor(img, x, y + 1);
                if (next != current) frontierList[{current, next}].push_back({(float)x, y + 0.5f});
            }
        }
    }

    world.provinceFrontiers = frontierList;
}

// ===============================================================================================================
// Adjacency Graph
// ===============================================================================================================

inline void buildAdjacency(World& world){
    const std::map<std::pair<uint32_t, uint32_t>,std::vector<SDL_FPoint>>& provinceFrontiers = world.provinceFrontiers;
    const std::list<Province>& provinces = world.provinces;
    std::map<int, std::vector<int>> adjacency;
    for (const auto& [pair, _] : provinceFrontiers) {
        Province* a = provinceFindByColor(provinces, pair.first);
        Province* b = provinceFindByColor(provinces, pair.second);
        if (!a || !b) continue;
        adjacency[a->id].push_back(b->id);
        adjacency[b->id].push_back(a->id);
    }
    world.adjacencyGraph = adjacency;
}

inline std::map<int, std::vector<int>> buildAccessibilityGraph(const World& world, const std::vector<std::string>& accessibleCountryTags){
    const std::unordered_set<std::string> accessible(
        accessibleCountryTags.begin(), accessibleCountryTags.end());

    std::unordered_map<int, Province*> idMap;
    for (auto& province : world.provinces)
        idMap[province.id] = const_cast<Province*>(&province);

    std::map<int, std::vector<int>> adjacency;

    for (const auto& [provinceId, neighbors] : world.adjacencyGraph) {
        auto itA = idMap.find(provinceId);
        if (itA == idMap.end() || !accessible.count(itA->second->owner)) continue;

        //std::cout << "Processing province: " << provinceId << " owner: " << itA->second->owner << std::endl;

        for (int neighborId : neighbors) {
            auto itB = idMap.find(neighborId);
            if (itB == idMap.end() || !accessible.count(itB->second->owner)) continue;

            adjacency[provinceId].push_back(neighborId);
        }
    }

    return adjacency;
}


inline void InitAllAccesibiltyGraphs(World& world){
    for (auto& country : world.countries) {
        country.accessibilityGraph = buildAccessibilityGraph(world, country.accessibleCountries);
    }
}

inline void rechargeAccesibilityGraph(World& world, Country* country){
        country -> accessibilityGraph = buildAccessibilityGraph(world, country -> accessibleCountries);
}

// ===============================================================================================================
// Frontiers
// ===============================================================================================================

inline void findFrontiersBetweenCountries( World& world) {
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>>& frontiers = world.provinceFrontiers;
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> filteredFrontiers;

    for (const auto& [key, points] : frontiers) {
        const Province* province1 = provinceFindByColor(world.provinces, key.first);
        const Province* province2 = provinceFindByColor(world.provinces, key.second);

        if (province1 == nullptr || province2 == nullptr) continue;

        const bool differentCountries = province1->owner != province2->owner
                                     && !province1->owner.empty()
                                     && !province2->owner.empty();

        const bool oneIsCountryOneIsNot = province1->owner.empty() != province2->owner.empty();

        if (differentCountries || oneIsCountryOneIsNot) {
            filteredFrontiers[key] = points;
        }
    }

    world.countryFrontiers = filteredFrontiers;
}

// ===============================================================================================================
// Provinces
// ===============================================================================================================

inline std::map<uint32_t, SDL_Point> initProvincesCenters(const World& world) {

    struct Accum {
        int sumX = 0;
        int sumY = 0;
        int count = 0;
    };

    std::map<uint32_t, Accum> accum;

    for (int y = 0; y < world.texHeight; y++) {

        for (int x = 0; x < world.texWidth; x++) {

            uint32_t color =
                getPixelColor(world.provincesBmp, x, y);

            auto& a = accum[color];

            a.sumX += x;
            a.sumY += y;
            a.count++;
        }
    }

    std::map<uint32_t, SDL_Point> centerList;

    for (auto& [color, a] : accum) {

        if (a.count > 0) {

            centerList[color] = {
                a.sumX / a.count,
                a.sumY / a.count
            };
        }
    }

    return centerList;
}

inline void prepareCountries(World& world) {
    SDL_Renderer* renderer = world.renderer;

    SDL_Surface* provinces = world.provincesBmp;
    if (!provinces) return;

    SDL_Surface* result = SDL_CreateRGBSurfaceWithFormat(
        0, provinces->w, provinces->h, 32, SDL_PIXELFORMAT_RGBA32
    );
    if (!result) return;

    SDL_FillRect(result, nullptr, SDL_MapRGBA(result->format, 0, 0, 0, 0));

    std::map<uint32_t, uint32_t> colorToCountryColor;

    SDL_LockSurface(provinces);
    SDL_LockSurface(result);

    int imgW = provinces->w;
    int imgH = provinces->h;

    for (int y = 0; y < imgH; y++) {
        for (int x = 0; x < imgW; x++) {
            uint32_t pixelColor = getPixelColor(provinces, x, y);

            auto it = colorToCountryColor.find(pixelColor);
            if (it != colorToCountryColor.end()) {
                if (it->second != 0) setPixel(result, x, y, it->second);
                continue;
            }

            Province* p = provinceFindByColor(world.provinces, pixelColor);
            if (!p || p->owner.empty()) {
                colorToCountryColor[pixelColor] = 0;
                continue;
            }

            Country* c = findCountryByTag(world.countries, p->owner);
            if (!c) {
                colorToCountryColor[pixelColor] = 0;
                continue;
            }

            uint32_t countryColor = SDL_MapRGB(result->format, c->color.r, c->color.g, c->color.b);

            colorToCountryColor[pixelColor] = countryColor;
            setPixel(result, x, y, countryColor);
        }
    }

    SDL_UnlockSurface(provinces);
    SDL_UnlockSurface(result);

    world.countriesImg = result;
}