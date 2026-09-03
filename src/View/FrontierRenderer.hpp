#pragma once

//=============================

#include "../utils.hpp"
#include "../Model/World.hpp"

//=============================

#include "SDL_rect.h"
#include "SDL_render.h"

//=============================

inline void renderPoints(
    World& world,  
    const std::map<std::pair<uint32_t,uint32_t>, std::vector<SDL_FPoint>>& pointList,
    SDL_Color color, 
    float size
){
    SDL_SetRenderDrawColor(world.renderer, color.r, color.g, color.b, color.a);

    for (const auto& [colorPair, points] : pointList) {
        for (const auto& point : points) {
            float sx = world.destRect.x + point.x * world.finalScale;
            float sy = world.destRect.y + point.y * world.finalScale;
            if (sx < 0 || sy < 0 || sx > world.winWidth || sy > world.winHeight) continue;
            SDL_FRect dot = { sx, sy, world.finalScale * size, world.finalScale * size };
        
            SDL_RenderFillRectF(world.renderer, &dot);
        }
    }
}

inline std::map<std::pair<uint32_t,uint32_t>, std::vector<SDL_FPoint>> filterFrontiersOfAProvince(World& world, int provinceId){

    std::map<std::pair<uint32_t,uint32_t>, std::vector<SDL_FPoint>> filtered;

    Province* p = provinceFindById(world.provinces, provinceId);

    if (!p) return filtered;

    uint32_t pColor = ((uint32_t)p->color.r << 16) | ((uint32_t)p->color.g << 8) | (uint32_t)p->color.b;

    for (const auto& [FrontierOwners, points] : world.provinceFrontiers) {
        if (FrontierOwners.first == pColor || FrontierOwners.second == pColor)
            filtered[FrontierOwners] = points;
    }
    return filtered;
}

inline void renderFrontiersAsPoints(
    World& world, 
    SDL_Color color,
    const std::map<std::pair<uint32_t,uint32_t>, 
    std::vector<SDL_FPoint>>& frontierList,
    float size
){
    renderPoints(world, frontierList, color, size);
}


inline void highligthProvinceFrontiers(World& world, SDL_Color color, int provinceId, float size = 1.0f) {
    
    std::map<std::pair<uint32_t,uint32_t>, std::vector<SDL_FPoint>> filtered = filterFrontiersOfAProvince(world, provinceId);

    renderPoints(world, filtered, color, size);
}




