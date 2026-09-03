#pragma once

//=============================

#include "../utils.hpp"
#include "../Model/World.hpp"

//=============================

#include "SDL_render.h"
#include <string>
#include <unordered_set>

//=============================
inline SDL_Texture* buildAccessibilityMap(World& world, const std::vector<std::string>& accessibleCountries) {
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

    SDL_Texture* result = SDL_CreateTextureFromSurface(world.renderer, dst);
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