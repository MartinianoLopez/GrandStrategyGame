#include "../utils.hpp"
#include "../Model/World.hpp"
#include "SDL_rect.h"
#include <algorithm>
#include <optional>
#include <chrono>


inline void renderText(
    SDL_Renderer* renderer,
    SDL_FRect rect,
    World& world,
    const std::string& fontId,
    int x,
    int y,
    const std::string& text
) {
    Font* font = nullptr;
    for (auto& f : world.fonts) {
        if (f.id == fontId) {
            font = &f;
            break;
        }
    }
    if (!font) return;

    for (char c : text) {
        unsigned char uc = static_cast<unsigned char>(c);

        if (uc >= 128)
            continue;

        Glyph& g = font->glyphs[(int)uc];

        if (!g.tex) continue;

        SDL_Rect dst = {
            x,
            y,
            g.w,
            g.h
        };

        SDL_RenderCopy(renderer, g.tex, nullptr, &dst);

        x += g.w;
    }
}
// ===============================================================================================================
// frontiers
// ===============================================================================================================
inline void renderFrontiers(
    World& world,
    SDL_Renderer* renderer,
    SDL_FRect destRect,
    SDL_Color color,
    int screenW,
    int screenH,
    const std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>>& frontierList,
    float size)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (const auto& [colorPair, points] : frontierList) {
        for (const auto& point : points) {
            float sx = destRect.x + point.x * world.finalScale;
            float sy = destRect.y + point.y * world.finalScale;
            if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue;
            SDL_FRect dot = { sx, sy, world.finalScale * size, world.finalScale * size };
            SDL_RenderFillRectF(renderer, &dot);
        }
    }
}

inline void markProvinceFrontiers(World& world, SDL_Renderer* renderer, SDL_FRect destRect, SDL_Color color, int provinceId) {
    Province* p = provinceFindById(world.provinces, provinceId);
    if (!p) return;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (const auto& [colorPair, points] : world.provinceFrontiers) {
        uint32_t pColor = ((uint32_t)p->color.r << 16) | ((uint32_t)p->color.g << 8) | (uint32_t)p->color.b;
        if (colorPair.first != pColor && colorPair.second != pColor) continue;
        for (const auto& point : points) {
            float sx = destRect.x + point.x * world.finalScale;
            float sy = destRect.y + point.y * world.finalScale;
            SDL_FRect dot = { sx, sy, world.finalScale, world.finalScale };
            SDL_RenderFillRectF(renderer, &dot);
        }
    }
}

// ===============================================================================================================
// display
// ===============================================================================================================
// do not use too expensive in terms of performance
inline void displaySurface(SDL_Renderer* renderer, SDL_Surface* surface, const SDL_FRect& destRect, Uint8 alpha) {
    //from surface to texture
    if (!surface) return;
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!converted) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, converted);
    SDL_FreeSurface(converted);
    // display texture 
    if (!texture) return;
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderCopyF(renderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
}

inline SDL_Texture* convertSurfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface) {
    if (!surface) return nullptr;
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!converted) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, converted);
    SDL_FreeSurface(converted);
    return texture;
}

inline void displayTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect& destRect, Uint8 alpha) {
    if (!texture) return;
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderCopyF(renderer, texture, nullptr, &destRect);
}

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
        renderArmy(renderer, world, "simple", (int)sx, (int)sy, army);
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

// ===============================================================================================================
// render
// ===============================================================================================================
inline void renderMap(World& world, SDL_Renderer* renderer, SDL_Window* window, bool isSecondMap) {
    auto t0 = std::chrono::high_resolution_clock::now();

    int winWidth, winHeight;
    SDL_GetWindowSize(window, &winWidth, &winHeight);

    world.finalScale = std::min(
        (float)winWidth  / world.texWidth,
        (float)winHeight / world.texHeight
    ) * world.scale;

    SDL_FRect destRect = {
        world.offsetX,
        world.offsetY,
        world.texWidth  * world.finalScale,
        world.texHeight * world.finalScale
    };

    if (isSecondMap) {
        destRect.x = world.offsetX - world.texWidth * world.finalScale; 
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    displayTexture(renderer, world.height,       destRect, 255);
    auto t2 = std::chrono::high_resolution_clock::now();

    displayTexture(renderer, world.terrain,      destRect, 200);
    auto t3 = std::chrono::high_resolution_clock::now();

    if (world.mapMode == "normal") {
        displayTexture(renderer, world.countriesTex, destRect, 245);
        auto t4 = std::chrono::high_resolution_clock::now();
    }
    if (world.mapMode == "access") {
        // displayTexture(renderer, world.countriesTex, destRect, 245); display texture access
        // auto t4 = std::chrono::high_resolution_clock::now();
    }
    
    if (world.mapMode == "diplomatic") {
        // generate and cache diplomatic texture for the designated country
        // displayTexture(renderer, world.countriesTex, destRect, 245); display diplomatic texture
        // auto t4 = std::chrono::high_resolution_clock::now();
    }

    if (world.scale > 6.0f)
        renderFrontiers(world, renderer, destRect, {0, 0, 0, 120}, winWidth, winHeight, world.provinceFrontiers, 1);
    auto t5 = std::chrono::high_resolution_clock::now();

    if (world.scale < 5.0f)
        renderFrontiers(world, renderer, destRect,{0, 0, 0, 220}, winWidth, winHeight, world.countryFrontiers, 6 / world.scale);
    auto t6 = std::chrono::high_resolution_clock::now();

    if (world.scale > 4.0f) {
        renderFrontiers(world, renderer, destRect, {0, 0, 0, 220}, winWidth, winHeight, world.countryFrontiers, 1);
        markProvinceFrontiers(world, renderer, destRect, {255, 255, 0, 240}, world.selectedProvince);
        renderArmies(world, renderer, destRect, winWidth, winHeight);
        showSelectedArmiesPaths(world, renderer, destRect);
    }
    auto t7 = std::chrono::high_resolution_clock::now();

    auto ms = [](auto a, auto b){ return std::chrono::duration<float, std::milli>(b - a).count(); };
    /*
    std::cerr << "[map" << (offset ? "_wrap" : "") << "]\n"
              << "  setup: "       << ms(t0,t1) << " ms\n"
              << "  height: "      << ms(t1,t2) << " ms\n"
              << "  terrain: "     << ms(t2,t3) << " ms\n"
              << "  countries: "   << ms(t3,t4) << " ms\n"
              << "  provFront: "   << ms(t4,t5) << " ms\n"
              << "  countryFront1: " << ms(t5,t6) << " ms\n"
              << "  countryFront2/marks/armies: " << ms(t6,t7) << " ms\n";
    */
}

inline void renderUI(SDL_Renderer* renderer, World& world, SDL_Window* window) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    for (auto& el : world.uiElements) {
        SDL_FRect rect = el.calculateBase(w, h);

        if (el.texture) {
            SDL_RenderCopyF(renderer, el.texture, nullptr, &rect);
        } else {
            SDL_SetRenderDrawColor(renderer, el.color.r, el.color.g, el.color.b, el.color.a);
            SDL_RenderFillRectF(renderer, &rect);
        }

        if (el.getText) {
            std::string text = el.getText();

            Font* font = nullptr;
            for (auto& f : world.fonts) {
                if (f.id == el.font) { font = &f; break; }
            }
            if (!font) continue;

            int tw = 0, th = 0;
            for (unsigned char c : text) {
                if (c >= 128) continue;
                Glyph& g = font->glyphs[c];
                tw += g.w;
                th = std::max(th, g.h);
            }

            int tx = (int)(rect.x + (rect.w - tw) * 0.5f);
            int ty = (int)(rect.y + (rect.h - th) * 0.5f);

            renderText(renderer, rect, world, el.font, tx, ty, text);
        }
    }
}

inline void renderGame(World& world, SDL_Renderer* renderer, SDL_Window* window) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    renderMap(world, renderer, window, false); // mapa principal
    renderMap(world, renderer, window, true);  // copia desplazada (wrapping)
    renderUI(renderer, world, window);  
    SDL_RenderPresent(renderer);
}
