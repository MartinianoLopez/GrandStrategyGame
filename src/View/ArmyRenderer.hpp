#pragma once

//=============================

#include "../utils.hpp"
#include "../Model/World.hpp"

//=============================

#include "SDL_rect.h"
#include "SDL_render.h"
#include <algorithm>
#include <optional>
#include <string>

// ===============================================================================================================
// Armies
// ===============================================================================================================

inline void renderSelectedOverlay(SDL_Renderer* renderer, World& world, int x, int y, const Army& army) {
    Font* font = nullptr;
    for (auto& f : world.fonts) {
        if (f.id == "simple") { font = &f; break; }
    }
    if (!font) return;
    std::string s = std::to_string(army.power);
    int totalW = 0, maxH = 0;
    for (char c : s) {
        Glyph& g = font->glyphs[(int)c];
        totalW += g.w;
        maxH = std::max(maxH, g.h);
    }
    const int padding = 1;
    int bx = (x - totalW / 2) - padding;
    int by = (y - maxH / 2) - padding;
    SDL_Rect border = { bx - 2, by - 2, totalW + padding * 2 + 4, maxH + padding * 2 + 4 };
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderDrawRect(renderer, &border);
}

inline void renderArmy(SDL_Renderer* renderer, World& world, const std::string& fontId, int x, int y, const Army& army) {
    // this is recreating the renderText function TODO replace it with the already working
    Font* font = nullptr;
    for (auto& f : world.fonts) {
        if (f.id == fontId) { font = &f; break; }
    }
    if (!font) return;
    std::string s = std::to_string(army.power);
    int totalW = 0, maxH = 0;
    for (char c : s) {
        Glyph& g = font->glyphs[(int)c];
        totalW += g.w;
        maxH = std::max(maxH, g.h);
    }
    x -= totalW / 2;
    y -= maxH / 2;
    const int padding = 1;
    SDL_Rect bg = { x - padding, y - padding, totalW + padding * 2, maxH + padding * 2 };
    SDL_SetRenderDrawColor(renderer, army.color.r, army.color.g, army.color.b, 180);
    SDL_RenderFillRect(renderer, &bg);
    for (char c : s) {
        Glyph& g = font->glyphs[(int)c];
        SDL_Rect dst = { x, y, g.w, g.h };
        SDL_RenderCopy(renderer, g.tex, nullptr, &dst);
        x += g.w;
    }
}

inline void renderArmies(World& world, SDL_Renderer* renderer, SDL_FRect destRect, int screenW, int screenH) {
    for (const auto& army : world.armies) {
        Province* province = provinceFindById(world.provinces, army.position);
        if (!province) continue;
        float sx = destRect.x + province->center.x * world.finalScale;
        float sy = destRect.y + province->center.y * world.finalScale;
        if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue;
        renderArmy(renderer, world, "army", (int)sx, (int)sy, army);
    }
    for (Army* army : world.selectedArmies) {
        if (!army) continue;
        Province* province = provinceFindById(world.provinces, army->position);
        if (!province) continue;
        float sx = destRect.x + province->center.x * world.finalScale;
        float sy = destRect.y + province->center.y * world.finalScale;
        if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue;
        renderSelectedOverlay(renderer, world, (int)sx, (int)sy, *army);
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

inline void showSelectedArmiesPaths(World& world, SDL_Renderer* renderer, SDL_FRect destRect) {
    for (Army* army : world.selectedArmies) {
        drawPath(army, world, renderer, destRect);
    }
}