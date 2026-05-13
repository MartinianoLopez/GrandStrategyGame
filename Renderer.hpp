#include "utils.hpp"
#include "World.hpp"
#include <algorithm>

// ===============================================================================================================
// frontiers
// ===============================================================================================================
void renderFrontiers(
    World& world,
    SDL_Renderer* renderer,
    SDL_Color color,
    int screenW,
    int screenH,
    const std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>>& frontierList,
    float size)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (const auto& [colorPair, points] : frontierList) {
        for (const auto& point : points) {
            float sx = world.offsetX + point.x * world.finalScale;
            float sy = world.offsetY + point.y * world.finalScale;
            if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue;
            SDL_FRect dot = { sx, sy, world.finalScale * size, world.finalScale * size };
            SDL_RenderFillRectF(renderer, &dot);
        }
    }
}

void markProvinceFrontiers(World& world, SDL_Renderer* renderer, SDL_Color color, int provinceId) {
    Province* p = provinceFindById(world.provinces, provinceId);
    if (!p) return;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (const auto& [colorPair, points] : world.provinceFrontiers) {
        uint32_t pColor = ((uint32_t)p->color.r << 16) | ((uint32_t)p->color.g << 8) | (uint32_t)p->color.b;
        if (colorPair.first != pColor && colorPair.second != pColor) continue;
        for (const auto& point : points) {
            float sx = world.offsetX + point.x * world.finalScale;
            float sy = world.offsetY + point.y * world.finalScale;
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
void renderText(
    SDL_Renderer* renderer,
    World& world,
    int x,
    int y,
    const std::string& text
) {
    for (char c : text) {

        if (c < 0 || c >= 128)
            continue;

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
void renderArmies(World& world, SDL_Renderer* renderer, int screenW, int screenH) {
    for (const auto& army : world.armies) {
        Province* p = provinceFindById(world.provinces, army.position);
        if (!p) continue;

        float sx = world.offsetX + p->center.x * world.finalScale;
        float sy = world.offsetY + p->center.y * world.finalScale;
        if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue;

        bool selected = (world.selectedProvince == army.position);
        renderArmy(renderer, world, (int)sx, (int)sy, army.power, selected);
    }
}

// ===============================================================================================================
// HUD
// ===============================================================================================================
void renderHUD(SDL_Renderer* renderer, World& world) {
    static SDL_Texture* flagTex  = nullptr;
    static std::string lastCountry;

    if (world.playerCountry != lastCountry) {
        if (flagTex) SDL_DestroyTexture(flagTex);
        std::string path = "assets/flags/" + world.playerCountry + ".tga";
        flagTex = IMG_LoadTexture(renderer, path.c_str());
        lastCountry = world.playerCountry;
    }

    Country* player = countryTagFind(world.countries, world.playerCountry);

    const int PANEL_H  = 48;
    const int FLAG_SIZE = 96;

    SDL_Rect panel = { 0, 0, 400, PANEL_H };
    SDL_SetRenderDrawColor(renderer, 26, 26, 26, 220);
    SDL_RenderFillRect(renderer, &panel);

    SDL_Rect flagRect = { 2, 2, FLAG_SIZE - 4, FLAG_SIZE - 4 };
    SDL_RenderCopy(renderer, flagTex, nullptr, &flagRect);

    SDL_SetRenderDrawColor(renderer, 245, 197, 24, 255);
    for (int i = 0; i < 3; i++) {
        SDL_Rect border = { 2 - i, 2 - i, FLAG_SIZE - 4 + i * 2, FLAG_SIZE - 4 + i * 2 };
        SDL_RenderDrawRect(renderer, &border);
    }

    const int ICON_SIZE = 22;
    const int ICON_X    = FLAG_SIZE + 8;
    const int ICON_Y    = (PANEL_H - ICON_SIZE) / 2;
    SDL_Rect iconRect = { ICON_X, ICON_Y, ICON_SIZE, ICON_SIZE };
    SDL_SetRenderDrawColor(renderer, 245, 197, 24, 255);
    SDL_RenderFillRect(renderer, &iconRect);

    int money = player ? player->money : 0;
    SDL_Color goldColor = { 245, 197, 24, 255 };
    renderText(renderer, world, ICON_X + ICON_SIZE + 6, (PANEL_H - 16) / 2,
               std::to_string(money));
}

// ===============================================================================================================
// render
// ===============================================================================================================
void render(World& world, SDL_Renderer* renderer, SDL_Window* window) {
    
    int winWidth, winHeight;
    SDL_GetWindowSize(window, &winWidth, &winHeight);
    world.finalScale = std::min(
        (float)winWidth  / world.texWidth,
        (float)winHeight / world.texHeight
    ) * world.scale;

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

        
 
    SDL_FRect destRect = {
        world.offsetX,
        world.offsetY,
        world.texWidth  * world.finalScale,
        world.texHeight * world.finalScale
    };
    displayTexture(renderer, world.height,  destRect, 255);
    displayTexture(renderer, world.terrain, destRect, 150);
    displaySurface(renderer, world.countriesImg, destRect, 240);

    if (world.scale > 6.0f)
        renderFrontiers(world, renderer, {0, 0, 0, 120}, winWidth, winHeight, world.provinceFrontiers, 1);

    if (world.scale < 5.0f)
        renderFrontiers(world, renderer, {0, 0, 0, 220}, winWidth, winHeight, world.countryFrontiers, 6 / world.scale);
        
    if (world.scale > 4.0f) {
        renderFrontiers(world, renderer, {0, 0, 0, 220}, winWidth, winHeight, world.countryFrontiers, 1);
        markProvinceFrontiers(world, renderer,{255, 255, 0, 240}, world.selectedProvince);
        renderArmies(world, renderer, winWidth, winHeight);
    }

    

    renderHUD(renderer, world);

    SDL_RenderPresent(renderer);
}