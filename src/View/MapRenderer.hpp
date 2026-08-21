#pragma once

//=============================

#include "../utils.hpp"
#include "ArmyRenderer.hpp"
#include "../Model/World.hpp"

//=============================

#include "SDL_rect.h"
#include "SDL_render.h"
#include <algorithm>
#include <string>
#include <unordered_set>

//=============================

inline void renderFrontiers(
    World& world,
    SDL_Renderer* renderer,
    SDL_FRect destRect,
    SDL_Color color,
    int screenW,
    int screenH,
    const std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>>& frontierList,
    float size
){
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

inline SDL_Texture* buildAccessibilityMap(World& world, SDL_Renderer* renderer, const std::vector<std::string>& accessibleCountries) {
    SDL_Surface* src = world.countriesImg;
    if (!src || !src->format) return nullptr;

    std::unordered_set<Uint32> accessibleColors;
    for (const auto& tag : accessibleCountries) {
        Country* c = findCountryByTag(world.countries, tag);
        if (c) accessibleColors.insert(colorToUint32(c->color, src->format));
    }

    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, src->w, src->h, 32, src->format->format);
    if (!dst) return nullptr;

    SDL_LockSurface(src);
    SDL_LockSurface(dst);

    Uint32* srcPixels = static_cast<Uint32*>(src->pixels);
    Uint32* dstPixels = static_cast<Uint32*>(dst->pixels);
    int totalPixels = src->w * src->h;
    Uint32 green       = SDL_MapRGB(dst->format, 0, 255, 0);
    Uint32 transparent = SDL_MapRGBA(dst->format, 255, 0, 0, 0);

    for (int i = 0; i < totalPixels; ++i)
        dstPixels[i] = accessibleColors.count(srcPixels[i]) ? green : transparent;

    SDL_UnlockSurface(dst);
    SDL_UnlockSurface(src);

    SDL_Texture* result = SDL_CreateTextureFromSurface(renderer, dst);
    SDL_FreeSurface(dst);
    return result;
}

inline SDL_Texture* buildDiplomaticMap(World& world, SDL_Renderer* renderer, const std::vector<Relationship> relationships) {
    SDL_Surface* src = world.countriesImg;
    if (!src || !src->format) return nullptr;

    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, src->w, src->h, 32, src->format->format);
    if (!dst) return nullptr;

    std::unordered_set<Uint32> countriesAtWar;

    for (const auto& relation : relationships) {
        Country* c = findCountryByTag(world.countries, relation.tag);
        if (c) countriesAtWar.insert(colorToUint32(c->color, src->format));
    }
    SDL_LockSurface(src);
    SDL_LockSurface(dst);

    Uint32* srcPixels = static_cast<Uint32*>(src->pixels);
    Uint32* dstPixels = static_cast<Uint32*>(dst->pixels);
    int totalPixels = src->w * src->h;
    Uint32 red       = SDL_MapRGB(dst->format, 255, 0, 0);
    Uint32 transparent = SDL_MapRGBA(dst->format, 0, 0, 0, 0);

    for (int i = 0; i < totalPixels; ++i)
        dstPixels[i] = countriesAtWar.count(srcPixels[i]) ? red : transparent;

    SDL_UnlockSurface(dst);
    SDL_UnlockSurface(src);

    SDL_Texture* result = SDL_CreateTextureFromSurface(renderer, dst);
    SDL_FreeSurface(dst);
    return result;
}

//=============================

inline void renderMap(World& world, bool isSecondMap) {
    SDL_Renderer* renderer = world.renderer;
    SDL_Window* window = world.window;

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

    displayTexture(renderer, world.height,       destRect, 255);

    displayTexture(renderer, world.terrain,      destRect, 200);

    if (world.mapMode == "normal") {
        displayTexture(renderer, world.countriesTex, destRect, 245);
        displayTexture(renderer, world.controlTex, destRect, 245);
    }
    
    if (world.mapMode == "access") {
        displayTexture(renderer, world.countriesTex, destRect, 100);
        if (!world.activeAccessibilityMap || world.selectedCountry != world.countryoftheAccesibilityMap) {
            
            SDL_DestroyTexture(world.activeAccessibilityMap);
            Country* country = findCountryByTag(world.countries, world.selectedCountry);
            if (country) {
                world.activeAccessibilityMap = buildAccessibilityMap(world, renderer, country->accessibleCountries);
            }
            world.countryoftheAccesibilityMap = world.selectedCountry;
        }
        if (world.activeAccessibilityMap)
            displayTexture(renderer, world.activeAccessibilityMap, destRect, 245);
    }
    
    
    if (world.mapMode == "diplomatic") {
        displayTexture(renderer, world.countriesTex, destRect, 100);

        if (!world.activeDiplomaticMap || world.selectedCountry != world.countryoftheAccesibilityMap) {
            
        SDL_DestroyTexture(world.activeDiplomaticMap);
        world.activeDiplomaticMap = nullptr;
        Country* country = findCountryByTag(world.countries, world.selectedCountry);
        if (country) {
        world.activeDiplomaticMap = buildDiplomaticMap(world, renderer, country->relationships);
                    }
        world.countryoftheAccesibilityMap = world.selectedCountry;
                }
        if (world.activeDiplomaticMap)
        displayTexture(renderer, world.activeDiplomaticMap, destRect, 245);
    }

    if (world.scale > 6.0f)
        renderFrontiers(world, renderer, destRect, {0, 0, 0, 120}, winWidth, winHeight, world.provinceFrontiers, 1);

    if (world.scale < 5.0f)
        renderFrontiers(world, renderer, destRect,{0, 0, 0, 220}, winWidth, winHeight, world.countryFrontiers, 6 / world.scale);

    if (world.scale > 4.0f) {
        renderFrontiers(world, renderer, destRect, {0, 0, 0, 220}, winWidth, winHeight, world.countryFrontiers, 1);
        markProvinceFrontiers(world, renderer, destRect, {255, 255, 0, 240}, world.selectedProvince);
        renderArmies(world, destRect, winWidth, winHeight);
        showSelectedArmiesPaths(world, renderer, destRect);
    }
}