#pragma once

//=============================

#include "../utils.hpp"
#include "ArmyRenderer.hpp"
#include "../Model/World.hpp"
#include "FrontierRenderer.hpp"

//=============================

#include "SDL_rect.h"
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


//=============================

inline void renderMap(World& world, bool isSecondMap) {
    SDL_Renderer* renderer = world.renderer;

    // second map is the offset map rendered to create the ilusion of a round globe
    if (isSecondMap) { 
        world.destRect.x = world.offsetX - world.texWidth * world.finalScale; 
    }

    displayTexture(world, world.height, 255);

    displayTexture(world, world.terrain, 200);

    if (world.mapMode == "normal") {
        displayTexture(world, world.countriesTex, 245);
        displayTexture(world, world.controlTex, 245);
    }
    
    if (world.mapMode == "access") {
        displayTexture(world, world.countriesTex, 100);
        if (!world.activeAccessibilityMap || world.selectedCountry != world.countryoftheAccesibilityMap) {
            
            SDL_DestroyTexture(world.activeAccessibilityMap);
            Country* country = findCountryByTag(world.countries, world.selectedCountry);
            if (country) {
                world.activeAccessibilityMap = buildAccessibilityMap(world, country->accessibleCountries);
            }
            world.countryoftheAccesibilityMap = world.selectedCountry;
        }
        if (world.activeAccessibilityMap)
            displayTexture(world, world.activeAccessibilityMap, 245);
    }
    
    
    if (world.mapMode == "diplomatic") {
        displayTexture(world, world.countriesTex, 100);

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
        displayTexture(world, world.activeDiplomaticMap, 245);
    }

    if (world.scale > 6.0f)
        renderFrontiersAsPoints(world, {0, 0, 0, 120}, world.provinceFrontiers, 1);

    if (world.scale < 5.0f)
        renderFrontiersAsPoints(world,{0, 0, 0, 220}, world.countryFrontiers, 6 / world.scale);

    if (world.scale > 4.0f) {
        renderFrontiersAsPoints(world, {0, 0, 0, 220}, world.countryFrontiers, 1);
        highligthProvinceFrontiers(world, {255, 255, 0, 240}, world.selectedProvince);
        renderArmies(world, world.destRect);
        showSelectedArmiesPaths(world, world.destRect);
    }
}