#pragma once
#include <map>
#include <list>
#include <optional>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include "World.hpp"

// ===============================================================================================================
// map find
// ===============================================================================================================
template <typename Map>
auto mapFind(const Map& map, const typename Map::key_type& key)
    -> std::optional<typename Map::mapped_type>
{
    auto it = map.find(key);
    if (it == map.end()) return std::nullopt;
    return it->second;
}

// ===============================================================================================================
// color
// ===============================================================================================================
std::string colorToString(uint32_t color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8)  & 0xFF;
    uint8_t b = (color)       & 0xFF;
    return std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b);
}

// ===============================================================================================================
// pixels
// ===============================================================================================================
uint32_t getPixelColor(SDL_Surface* surface, int x, int y) {

    uint8_t* pixel =
        (uint8_t*)surface->pixels
        + y * surface->pitch
        + x * surface->format->BytesPerPixel;

    uint32_t raw = *(uint32_t*)pixel;

    uint8_t r, g, b;

    SDL_GetRGB(
        raw,
        surface->format,
        &r,
        &g,
        &b
    );

    return
        ((uint32_t)r << 16) |
        ((uint32_t)g << 8) |
        (uint32_t)b;
}

void setPixel(SDL_Surface* surface, int x, int y, uint32_t color) {
    uint8_t r = (color)       & 0xFF;
    uint8_t g = (color >> 8)  & 0xFF;
    uint8_t b = (color >> 16) & 0xFF;
    Uint32 pixel = SDL_MapRGBA(surface->format, b, g, r, 255);
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * 4;
    *(Uint32*)p = pixel;
}

// ===============================================================================================================
// surface → texture
// ===============================================================================================================
SDL_Texture* surfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface) {
    if (!surface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surface);
    return texture;
}

// ===============================================================================================================
// province find by bmp color
// ===============================================================================================================


Country* countryTagFind(const std::list<Country>& list, const std::string& tag) {
    for (auto& country : list)
        if (country.tag == tag)
            return const_cast<Country*>(&country);
    return nullptr;
}

Army* armyPositionFind(const std::list<Army>& list, int provinceId) {
    for (auto& army : list)
        if (army.position == provinceId)
            return const_cast<Army*>(&army);
    return nullptr;
}

// ===============================================================================================================
// armies
// ===============================================================================================================
void moveArmy(Army& army, int toProvinceId) {
    army.position = toProvinceId;
}
Province* provinceFindById(const std::list<Province>& list, int id) {
    for (auto& province : list) {
        if (province.id == id)
            return const_cast<Province*>(&province);
    }

    return nullptr;
}
Province* provinceFindByColor(const std::list<Province>& list, uint32_t color) {
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8)  & 0xFF;
    int b = (color)       & 0xFF;

    for (auto& province : list) {
        if (province.color.r == r &&
            province.color.g == g &&
            province.color.b == b) {
            return const_cast<Province*>(&province);
        }
    }
    return nullptr;
}