#pragma once

//=============================

#include "../utils.hpp"
#include "../Model/World.hpp"
#include "../Model/FontLoader.hpp"

//=============================

#include "SDL_rect.h"
#include "SDL_render.h"
#include <optional>
#include <string>

// ===============================================================================================================
// Armies
// ===============================================================================================================

inline void renderSelectedOverlay(World& world, int x, int y, const Army& army) {
    std::string s = std::to_string(army.power);

    TextCache& cache = getOrRenderText(world, "simple", s);
    if (!cache.texture) return;

    const int padding = 1;
    int bx = (x - cache.w / 2) - padding;
    int by = (y - cache.h / 2) - padding;

    SDL_Rect border = { bx - 2, by - 2, cache.w + padding * 2 + 4, cache.h + padding * 2 + 4 };
    SDL_SetRenderDrawColor(world.renderer, 255, 255, 0, 255);
    SDL_RenderDrawRect(world.renderer, &border);
}

inline void renderArmy(World& world, int x, int y, const Army& army) {
    std::string s = std::to_string(army.power);

    TextCache& cache = getOrRenderText(world, "army", s);
    if (!cache.texture) return;

    x -= cache.w / 2;
    y -= cache.h / 2;

    const int padding = 1;
    SDL_Rect bg = { x - padding, y - padding, cache.w + padding * 2, cache.h + padding * 2 };
    SDL_SetRenderDrawColor(world.renderer, army.color.r, army.color.g, army.color.b, 180);
    SDL_RenderFillRect(world.renderer, &bg);

    SDL_Rect dst = { x, y, cache.w, cache.h };
    SDL_RenderCopy(world.renderer, cache.texture, nullptr, &dst);
}

inline void renderArmies(World& world, SDL_FRect destRect) {
    for (const auto& army : world.armies) {
        Province* province = provinceFindById(world.provinces, army.position);
        if (!province) continue;
        float sx = destRect.x + province->center.x * world.finalScale;
        float sy = destRect.y + province->center.y * world.finalScale;
        if (sx < 0 || sy < 0 || sx > world.winWidth || sy > world.winHeight) continue;
        renderArmy(world, (int)sx, (int)sy, army);
    }
    for (Army* army : world.selectedArmies) {
        if (!army) continue;
        Province* province = provinceFindById(world.provinces, army->position);
        if (!province) continue;
        float sx = destRect.x + province->center.x * world.finalScale;
        float sy = destRect.y + province->center.y * world.finalScale;
        if (sx < 0 || sy < 0 || sx > world.winWidth || sy > world.winHeight) continue;
        renderSelectedOverlay(world, (int)sx, (int)sy, *army);
    }
}

// ============================================================
// army path
// ============================================================

inline std::optional<Army> findArmy(World& world, int selectedProvince) {
    for (const auto& army : world.armies) {
        if (army.position == selectedProvince)
            return army;
    }
    return std::nullopt;
}

inline void drawPath(Army* army, World& world, SDL_Renderer* renderer, SDL_FRect destRect) {
    if (!army || army->path.empty()) return;
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    Province* from = provinceFindById(world.provinces, army->position);
    if (!from) return;
    float px = destRect.x + from->center.x * world.finalScale;
    float py = destRect.y + from->center.y * world.finalScale;
    for (int provinceId : army->path) {
        Province* p = provinceFindById(world.provinces, provinceId);
        if (!p) continue;
        float cx = destRect.x + p->center.x * world.finalScale;
        float cy = destRect.y + p->center.y * world.finalScale;
        SDL_RenderDrawLineF(renderer, px, py, cx, cy);
        px = cx;
        py = cy;
    }
}

inline void showSelectedArmiesPaths(World& world, SDL_FRect destRect) {
    SDL_Renderer* renderer = world.renderer;
    for (Army* army : world.selectedArmies) {
        drawPath(army, world, renderer, destRect);
    }
}