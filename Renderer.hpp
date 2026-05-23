#include "utils.hpp"
#include "World.hpp"
#include <algorithm>

// ===============================================================================================================
// frontiers
// ===============================================================================================================
void renderFrontiers(
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

void markProvinceFrontiers(World& world, SDL_Renderer* renderer, SDL_FRect destRect, SDL_Color color, int provinceId) {
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
void displaySurface(SDL_Renderer* renderer, SDL_Surface* surface, const SDL_FRect& destRect, Uint8 alpha) {
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) return;
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderCopyF(renderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
}

void displayTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect& destRect, Uint8 alpha) {
    if (!texture) return;
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderCopyF(renderer, texture, nullptr, &destRect);
}

// ===============================================================================================================
// text & numbers
// ===============================================================================================================

void renderArmy(
    SDL_Renderer* renderer,
    World& world,
    int x,
    int y,
    int number,
    bool selected
) {
    std::string s = std::to_string(number);

    int totalW = 0;
    int maxH = 0;

    for (char c : s) {
        Glyph& g = world.glyphs[(int)c];
        totalW += g.w;
        maxH = std::max(maxH, g.h);
    }

    x -= totalW / 2;
    y -= maxH / 2;

    x = int(x);
    y = int(y);

    const int padding = 1;

    SDL_Rect bg = {
        x - padding,
        y - padding,
        totalW + padding * 2,
        maxH + padding * 2
    };

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
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

        Glyph& g = world.glyphs[(int)c];

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
void renderArmies(World& world, SDL_Renderer* renderer, SDL_FRect destRect, int screenW, int screenH) {
    for (const auto& army : world.armies) {
        Province* p = provinceFindById(world.provinces, army.position);
        if (!p) continue;
        float sx = destRect.x + p->center.x * world.finalScale;
        float sy = destRect.y + p->center.y * world.finalScale;
        if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue;
        bool selected = (world.selectedProvince == army.position);
        renderArmy(renderer, world, (int)sx, (int)sy, army.power, selected);
    }
}

// ===============================================================================================================
// render
// ===============================================================================================================
void renderMap(World& world, SDL_Renderer* renderer, SDL_Window* window, bool offset) {
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

    if (offset) {
        destRect.x = world.offsetX - world.texWidth * world.finalScale; 
    }

    displayTexture(renderer, world.height,       destRect, 255);
    displayTexture(renderer, world.terrain,      destRect, 150);
    displaySurface(renderer, world.countriesImg, destRect, 240);

    if (world.scale > 6.0f)
        renderFrontiers(world, renderer, destRect, {0, 0, 0, 120}, winWidth, winHeight, world.provinceFrontiers, 1);
    if (world.scale < 5.0f)
        renderFrontiers(world, renderer, destRect,{0, 0, 0, 220}, winWidth, winHeight, world.countryFrontiers, 6 / world.scale);
    if (world.scale > 4.0f) {
        renderFrontiers(world, renderer, destRect, {0, 0, 0, 220}, winWidth, winHeight, world.countryFrontiers, 1);
        markProvinceFrontiers(world, renderer, destRect, {255, 255, 0, 240}, world.selectedProvince);
        renderArmies(world, renderer, destRect, winWidth, winHeight);
    }
}

void renderUI(SDL_Renderer* renderer, World& world, SDL_Window* window) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    for (auto& el : world.uiElements) {
        SDL_FRect r = el.resolve(w, h);

        if (el.texture) {
            SDL_RenderCopyF(renderer, el.texture, nullptr, &r);
        } else {
            SDL_SetRenderDrawColor(renderer, el.color.r, el.color.g, el.color.b, el.color.a);
            SDL_RenderFillRectF(renderer, &r);
        }

        if (el.getText) {
            std::string text = el.getText();
            int tw, th;
            TTF_SizeText(world.font, text.c_str(), &tw, &th);
            float tx = r.x + (r.w - tw) * 0.5f;
            float ty = r.y + (r.h - th) * 0.5f;
            renderText(renderer, world, tx, ty, text);
        }
    }
}


void renderGame(World& world, SDL_Renderer* renderer, SDL_Window* window) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    renderMap(world, renderer, window, false); // mapa principal
    renderMap(world, renderer, window, true);  // copia desplazada (wrapping)
    renderUI(renderer, world, window);  
    SDL_RenderPresent(renderer);
}
