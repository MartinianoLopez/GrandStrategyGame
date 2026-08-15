#pragma once

//============================

#include "../Model/World.hpp"
#include "../Model/FontLoader.hpp"
//============================
#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>

// Draws text centered inside rect, using the cache above
inline void renderElementText(World& world, const UIElement& element, const SDL_FRect& rect) {
    if (!element.textProvider) return;

    std::string text = element.textProvider();
    if (text.empty()) return;

    TextCache& cache = getOrRenderText(world, element.font, text);
    if (!cache.texture) return;

    SDL_FRect dst = {
        rect.x + (rect.w - cache.w) * 0.5f,
        rect.y + (rect.h - cache.h) * 0.5f,
        (float)cache.w,
        (float)cache.h
    };

    SDL_RenderCopyF(world.renderer, cache.texture, nullptr, &dst);
}