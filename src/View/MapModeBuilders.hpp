#pragma once

//=============================

#include "../utils.hpp"
#include "../Model/World.hpp"
#include "../Simulation/Diplomacy.hpp"

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


inline SDL_Texture* buildDiplomaticMap(World& world, SDL_Renderer* renderer, const std::string& tag) {
    SDL_Surface* src = world.countriesImg;
    if (!src || !src->format) return nullptr;

    SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, src->w, src->h, 32, src->format->format);
    if (!dst) return nullptr;

    std::unordered_set<Uint32> countriesAtWar;
    std::unordered_set<Uint32> alliedCountries;
    Uint32 selectedCountryColor = 0;
    if (Country* selectedCountry = findCountryByTag(world.countries, tag)) {
        selectedCountryColor = colorToUint32(selectedCountry->color, src->format);
        for (const auto& allyTag : getAllies(*selectedCountry)) {
            Country* ally = findCountryByTag(world.countries, allyTag);
            if (ally) alliedCountries.insert(colorToUint32(ally->color, src->format));
        }
    }
    for (const auto& c : world.countries)
        if (c.tag != tag && isAtWar(world, tag, c.tag))
            countriesAtWar.insert(colorToUint32(c.color, src->format));

    SDL_LockSurface(src);
    SDL_LockSurface(dst);

    Uint32* srcPixels = static_cast<Uint32*>(src->pixels);
    Uint32* dstPixels = static_cast<Uint32*>(dst->pixels);
    int totalPixels = src->w * src->h;
    Uint32 red         = SDL_MapRGB(dst->format, 255, 0, 0);
    Uint32 blue        = SDL_MapRGB(dst->format, 0, 0, 255);
    Uint32 lightBlue   = SDL_MapRGB(dst->format, 74, 165, 212);
    Uint32 transparent = SDL_MapRGBA(dst->format, 0, 0, 0, 0);

    for (int i = 0; i < totalPixels; ++i) {
        if (srcPixels[i] == selectedCountryColor) dstPixels[i] = blue;
        else if (alliedCountries.count(srcPixels[i])) dstPixels[i] = lightBlue;
        else if (countriesAtWar.count(srcPixels[i])) dstPixels[i] = red;
        else dstPixels[i] = transparent;
    }

    SDL_UnlockSurface(dst);
    SDL_UnlockSurface(src);

    SDL_Texture* result = SDL_CreateTextureFromSurface(renderer, dst);
    SDL_FreeSurface(dst);
    return result;
}

inline SDL_Texture* buildPeaceTreatyMap(World& world) {
    SDL_Surface* source = world.provincesBmp;
    if (!source || !source->format) return nullptr;

    const std::vector<std::string>* playerSide = nullptr;
    const std::vector<std::string>* enemySide = nullptr;
    for (const auto& war : world.activeWars) {
        const auto contains = [](const std::vector<std::string>& countries, const std::string& tag) {
            return std::find(countries.begin(), countries.end(), tag) != countries.end();
        };
        if (contains(war.attackerCountries, world.playerCountry) && contains(war.defenderCountries, world.peaceTreatyTarget)) {
            playerSide = &war.attackerCountries;
            enemySide = &war.defenderCountries;
            break;
        }
        if (contains(war.defenderCountries, world.playerCountry) && contains(war.attackerCountries, world.peaceTreatyTarget)) {
            playerSide = &war.defenderCountries;
            enemySide = &war.attackerCountries;
            break;
        }
    }
    if (!playerSide || !enemySide) return nullptr;

    const auto hasTag = [](const std::vector<std::string>& tags, const std::string& tag) {
        return std::find(tags.begin(), tags.end(), tag) != tags.end();
    };
    SDL_Surface* destination = SDL_CreateRGBSurfaceWithFormat(
        0, source->w, source->h, 32, SDL_PIXELFORMAT_RGBA32
    );
    if (!destination) return nullptr;

    const Uint32 playerGreen = SDL_MapRGB(destination->format, 36, 150, 72);
    const Uint32 offerableGreen = SDL_MapRGB(destination->format, 130, 220, 145);
    const Uint32 occupiedGreen = SDL_MapRGB(destination->format, 130, 220, 145);
    const Uint32 enemyRed = SDL_MapRGB(destination->format, 205, 64, 64);
    const Uint32 neutralGray = SDL_MapRGB(destination->format, 190, 195, 198);
    const Uint32 transparent = SDL_MapRGBA(destination->format, 0, 0, 0, 0);
    const auto& activeProvinceList = world.peaceTreatyMode == PeaceTreatyMode::DEMAND
        ? world.peaceTreatyDemands : world.peaceTreatyOffers;
    const std::unordered_set<int> selectedProvinces(activeProvinceList.begin(), activeProvinceList.end());

    std::unordered_map<uint32_t, Uint32> provinceColors;
    provinceColors.reserve(world.provinces.size() * 2);
    world.peaceTreatyBaseColors.clear();
    world.peaceTreatyBaseColors.reserve(world.provinces.size());
    world.peaceTreatyPlayerColor = playerGreen;
    world.peaceTreatySelectionColor = world.peaceTreatyMode == PeaceTreatyMode::OFFER ? enemyRed : playerGreen;
    for (const auto& province : world.provinces) {
        Uint32 color = transparent;
        if (province.terrainType != TerrainType::OCEAN &&
            !province.owner.empty() && province.owner != "NONE") {
            color = neutralGray;
        }
        if (province.terrainType == TerrainType::OCEAN ||
            province.owner.empty() || province.owner == "NONE") {
            color = transparent;
        } else if (selectedProvinces.count(province.id)) {
            color = playerGreen;
        } else if (hasTag(*playerSide, province.owner)) {
            color = world.peaceTreatyMode == PeaceTreatyMode::OFFER && province.owner == world.playerCountry
                ? offerableGreen : playerGreen;
        } else if (hasTag(*enemySide, province.owner)) {
            color = world.peaceTreatyMode == PeaceTreatyMode::DEMAND && hasTag(*playerSide, province.controller)
                ? occupiedGreen : enemyRed;
        }
        const uint32_t provinceColor = (static_cast<uint32_t>(province.color.r) << 16) |
                                       (static_cast<uint32_t>(province.color.g) << 8) |
                                       static_cast<uint32_t>(province.color.b);
        provinceColors[provinceColor] = color;
        world.peaceTreatyBaseColors[province.id] = color;
    }

    if (SDL_MUSTLOCK(source)) SDL_LockSurface(source);
    if (SDL_MUSTLOCK(destination)) SDL_LockSurface(destination);
    for (int y = 0; y < source->h; ++y) {
        for (int x = 0; x < source->w; ++x) {
            const auto province = provinceColors.find(getPixelColor(source, x, y));
            setPixel(destination, x, y, province == provinceColors.end() ? transparent : province->second);
        }
    }
    if (SDL_MUSTLOCK(destination)) SDL_UnlockSurface(destination);
    if (SDL_MUSTLOCK(source)) SDL_UnlockSurface(source);

    SDL_FreeSurface(world.peaceTreatyMapSurface);
    world.peaceTreatyMapSurface = destination;
    world.peaceTreatyRenderedSelections = selectedProvinces;
    return SDL_CreateTextureFromSurface(world.renderer, destination);
}

inline void updatePeaceTreatyProvinceColor(World& world, int provinceId, bool selected) {
    if (!world.peaceTreatyMap || !world.peaceTreatyMapSurface) return;
    Province* province = findProvinceById(world.provinces, provinceId);
    auto baseColor = world.peaceTreatyBaseColors.find(provinceId);
    if (!province || baseColor == world.peaceTreatyBaseColors.end()) return;

    const Uint32 color = selected ? world.peaceTreatySelectionColor : baseColor->second;
    if (province->shape.empty()) return;
    int minX = world.peaceTreatyMapSurface->w;
    int minY = world.peaceTreatyMapSurface->h;
    int maxX = 0;
    int maxY = 0;
    for (const auto& [x, y] : province->shape) {
        setPixel(world.peaceTreatyMapSurface, x, y, color);
        minX = std::min(minX, static_cast<int>(x));
        minY = std::min(minY, static_cast<int>(y));
        maxX = std::max(maxX, static_cast<int>(x));
        maxY = std::max(maxY, static_cast<int>(y));
    }

    SDL_Rect rect{minX, minY, maxX - minX + 1, maxY - minY + 1};
    const Uint8* pixels = static_cast<const Uint8*>(world.peaceTreatyMapSurface->pixels)
                        + minY * world.peaceTreatyMapSurface->pitch
                        + minX * world.peaceTreatyMapSurface->format->BytesPerPixel;
    SDL_UpdateTexture(world.peaceTreatyMap, &rect, pixels, world.peaceTreatyMapSurface->pitch);
}

inline void refreshPeaceTreatyDemandColors(World& world) {
    const auto& activeProvinceList = world.peaceTreatyMode == PeaceTreatyMode::DEMAND
        ? world.peaceTreatyDemands : world.peaceTreatyOffers;
    const std::unordered_set<int> currentSelections(activeProvinceList.begin(), activeProvinceList.end());
    for (int provinceId : world.peaceTreatyRenderedSelections) {
        if (!currentSelections.count(provinceId)) updatePeaceTreatyProvinceColor(world, provinceId, false);
    }
    for (int provinceId : currentSelections) {
        if (!world.peaceTreatyRenderedSelections.count(provinceId)) updatePeaceTreatyProvinceColor(world, provinceId, true);
    }
    world.peaceTreatyRenderedSelections = currentSelections;
}