#include "../utils.hpp"
#include "../Model/World.hpp"
#include <algorithm>
#include <chrono>

inline void renderText(
    SDL_Renderer* renderer,
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
// text & numbers
// ===============================================================================================================

inline void renderArmy(
    SDL_Renderer* renderer,
    World& world,
    const std::string& fontId,
    int x,
    int y,
    Army army,
    bool selected
) {
    Font* font = nullptr;
    for (auto& f : world.fonts) {
        if (f.id == fontId) { font = &f; break; }
    }
    if (!font) return;

    std::string s = std::to_string(army.power);

    int totalW = 0;
    int maxH = 0;

    for (char c : s) {
        Glyph& g = font->glyphs[(int)c];
        totalW += g.w;
        maxH = std::max(maxH, g.h);
    }

    x -= totalW / 2;
    y -= maxH / 2;

    const int padding = 1;

    SDL_Rect bg = {
        x - padding,
        y - padding,
        totalW + padding * 2,
        maxH + padding * 2
    };

    SDL_SetRenderDrawColor(renderer, army.color.r, army.color.g, army.color.b, 180);
    SDL_RenderFillRect(renderer, &bg);

    if (selected) {
        SDL_Rect border = {
            bg.x - 2,
            bg.y - 2,
            bg.w + 4,
            bg.h + 4
        };

        SDL_SetRenderDrawColor(renderer, 255,255,0,255);
        SDL_RenderDrawRect(renderer, &border);
    }

    for (char c : s) {

        Glyph& g = font->glyphs[(int)c];

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
// armies
// ===============================================================================================================
inline void renderArmies(World& world, SDL_Renderer* renderer, SDL_FRect destRect, int screenW, int screenH) {
    for (const auto& army : world.armies) {
        Province* p = provinceFindById(world.provinces, army.position);
        if (!p) continue;
        float sx = destRect.x + p->center.x * world.finalScale;
        float sy = destRect.y + p->center.y * world.finalScale;
        if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue;
        bool selected = (world.selectedProvince == army.position);
        renderArmy(renderer, world, "simple", (int)sx, (int)sy, army, selected);
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

    displayTexture(renderer, world.countriesTex, destRect, 245);
    auto t4 = std::chrono::high_resolution_clock::now();

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
        SDL_FRect r = el.calculateBase(w, h);

        if (el.texture) {
            SDL_RenderCopyF(renderer, el.texture, nullptr, &r);
        } else {
            SDL_SetRenderDrawColor(renderer, el.color.r, el.color.g, el.color.b, el.color.a);
            SDL_RenderFillRectF(renderer, &r);
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

            int tx = (int)(r.x + (r.w - tw) * 0.5f);
            int ty = (int)(r.y + (r.h - th) * 0.5f);

            renderText(renderer, world, el.font, tx, ty, text);
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
