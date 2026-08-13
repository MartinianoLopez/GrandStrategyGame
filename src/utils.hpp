#pragma once

//=================================

#include "Model/World.hpp"

//=================================

#include <list>
#include <string>
#include <SDL2/SDL.h>

// ===============================================================================================================
// surface → texture
// ===============================================================================================================

inline SDL_Texture* surfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface) {
    if (!surface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    return texture;
}

inline SDL_Color getPixel(SDL_Surface* surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    uint32_t pixel;
    switch (bpp) {
        case 1: pixel = *p; break;
        case 2: pixel = *(uint16_t*)p; break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixel = p[0] << 16 | p[1] << 8 | p[2];
            else
                pixel = p[0] | p[1] << 8 | p[2] << 16;
            break;
        case 4: pixel = *(uint32_t*)p; break;
        default: pixel = 0;
    }

    SDL_Color color;
    SDL_GetRGB(pixel, surface->format, &color.r, &color.g, &color.b);
    color.a = 255;
    return color;
}

// ===============================================================================================================
// color (used only for debuging)
// ===============================================================================================================

inline std::string colorToString(uint32_t color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8)  & 0xFF;
    uint8_t b = (color)       & 0xFF;
    return std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b);
}

inline Uint32 colorToUint32(const Color color, SDL_PixelFormat* format) {
    return SDL_MapRGB(format, color.r, color.g, color.b);
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

// ===============================================================================================================
// Texture helpers
// ===============================================================================================================

inline void displayTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect& destRect, Uint8 alpha) {
    if (!texture) return;
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderCopyF(renderer, texture, nullptr, &destRect);
}

inline SDL_Texture* convertSurfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface) {
    if (!surface) return nullptr;
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!converted) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, converted);
    SDL_FreeSurface(converted);
    return texture;
}