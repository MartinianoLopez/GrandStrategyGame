#pragma once
#include <list>
#include <string>
#include <SDL2/SDL.h>
#include "Model/World.hpp"

// ===============================================================================================================
// color (used only for debuging)
// ===============================================================================================================

inline std::string colorToString(uint32_t color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8)  & 0xFF;
    uint8_t b = (color)       & 0xFF;
    return std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b);
}

// ===============================================================================================================
// pixels (used for loading assets and for province picking)
// ===============================================================================================================

inline uint32_t getPixelColor(SDL_Surface* surface, int x, int y) {

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

// ===============================================================================================================
// used to prepare countries map
// ===============================================================================================================
inline void setPixel(SDL_Surface* surface, int x, int y, uint32_t color) {
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
    *(Uint32*)p = color;
}

// ===============================================================================================================
// country finder
// ===============================================================================================================

inline Country* findCountryByTag(const std::list<Country>& list, const std::string& tag) {
    for (auto& country : list)
        if (country.tag == tag)
            return const_cast<Country*>(&country);
    return nullptr;
}

// ===============================================================================================================
// province finders
// ===============================================================================================================

inline Province* provinceFindById(const std::list<Province>& list, int id) {
    for (auto& province : list) {
        if (province.id == id)
            return const_cast<Province*>(&province);
    }

    return nullptr;
}

inline Province* provinceFindByColor(const std::list<Province>& list, uint32_t color) {
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
