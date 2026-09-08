#pragma once

//=============================

#include "../utils.hpp"
#include "../Model/World.hpp"
//=============================

#include "SDL_rect.h"
#include "SDL_render.h"
#include <string>


//=============================

inline void renderFrontierStyle(
    World& world,
    const std::map<std::pair<uint32_t,uint32_t>, FrontierData>& frontiers
){
    std::vector<SDL_Vertex> sdlVerts;

    for (const auto& [key, data] : frontiers) {
        // culling: transformar bounds mundo -> pantalla
        float sx0 = world.destRect.x + data.minX * world.finalScale;
        float sy0 = world.destRect.y + data.minY * world.finalScale;
        float sx1 = world.destRect.x + data.maxX * world.finalScale;
        float sy1 = world.destRect.y + data.maxY * world.finalScale;

        if (sx1 < 0 || sy1 < 0 || sx0 > world.winWidth || sy0 > world.winHeight) continue;

        for (auto& v : data.cachedVerts) {
            SDL_Vertex sv = v;
            sv.position.x = world.destRect.x + v.position.x * world.finalScale;
            sv.position.y = world.destRect.y + v.position.y * world.finalScale;
            sdlVerts.push_back(sv);
        }
    }

    if (!sdlVerts.empty())
        SDL_RenderGeometry(world.renderer, nullptr, sdlVerts.data(), (int)sdlVerts.size(), nullptr, 0);
}


inline std::map<std::pair<uint32_t,uint32_t>, FrontierData> filterCachedFrontiersOfAProvince(
    World& world, const std::string& styleName, int provinceId
){
    std::map<std::pair<uint32_t,uint32_t>, FrontierData> filtered;

    Province* p = provinceFindById(world.provinces, provinceId);
    if (!p) return filtered;

    uint32_t pColor = ((uint32_t)p->color.r << 16) | ((uint32_t)p->color.g << 8) | (uint32_t)p->color.b;

    auto it = world.frontierCache.find(styleName);
    if (it == world.frontierCache.end()) return filtered;

    for (const auto& [key, data] : it->second.frontiers) {
        if (key.first == pColor || key.second == pColor)
            filtered[key] = data;
    }
    return filtered;
}

inline void highligthProvinceFrontiers(World& world, int provinceId, const std::string& styleName) {
    auto filtered = filterCachedFrontiersOfAProvince(world, styleName, provinceId);
    renderFrontierStyle(world, filtered);
}

inline void renderSmoothFrontiers(World &world) {
    if (world.scale > 6.0f){
        renderFrontierStyle(world, world.frontierCache["province_frontiers"].frontiers);
    }

    if (world.scale > 5.0f){
        renderFrontierStyle(world, world.frontierCache["country_frontiers_thick"].frontiers);
    }

    if (world.scale > 4.0f) {
        renderFrontierStyle(world, world.frontierCache["country_frontiers_thin"].frontiers);
        renderFrontierStyle(world, filterCachedFrontiersOfAProvince(world, "highlight", world.selectedProvince));
    }
}