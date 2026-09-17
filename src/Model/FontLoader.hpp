#pragma once

#include "World.hpp"

inline Font initFont(SDL_Renderer* renderer, const std::string& id, const std::string& path, SDL_Color color, int size) {
    TTF_Font* font = TTF_OpenFont(path.c_str(), size);
    return { id, font, color };
}

inline void initFonts(World& world) {
    SDL_Renderer* renderer = world.renderer;

    world.fonts.push_back(initFont(renderer, "army",   "assets/fonts/Cinzel/static/Cinzel-SemiBold.ttf", {0, 0, 0, 255}, 10));
    world.fonts.push_back(initFont(renderer, "simple", "assets/fonts/Cinzel/static/Cinzel-SemiBold.ttf", {0, 0, 0, 255}, 20));
    world.fonts.push_back(initFont(renderer, "fancy",  "assets/fonts/Cinzel/static/Cinzel-SemiBold.ttf", {220, 220, 220, 255}, 22));

    world.fonts.push_back(initFont(renderer, "country_small",  "assets/fonts/Sedan_SC/SedanSC-Regular.ttf", {255, 255, 255, 255}, 12));
    world.fonts.push_back(initFont(renderer, "country_medium", "assets/fonts/Sedan_SC/SedanSC-Regular.ttf", {255, 255, 255, 255}, 16));
    world.fonts.push_back(initFont(renderer, "country_large",  "assets/fonts/Sedan_SC/SedanSC-Regular.ttf", {255, 255, 255, 255}, 22));
}

inline Font* findFont(World& world, const std::string& fontId) {
    for (auto& font : world.fonts) {
        if (font.id == fontId) return &font;
    }
    return nullptr;
}

// Render text once per fontId + text combo and reuse it afterwards.
inline TextCache& getOrRenderText(World& world, const std::string& fontId, const std::string& text) {
    std::string key = fontId + "|" + text;

    auto it = world.ui.textCache.find(key);
    if (it != world.ui.textCache.end()) return it->second;

    Font* font = findFont(world, fontId);
    TextCache cache;

    if (font && font->font) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font->font, text.c_str(), font->color);
        if (surface) {
            cache.texture = SDL_CreateTextureFromSurface(world.renderer, surface);
            cache.w = surface->w;
            cache.h = surface->h;
            SDL_FreeSurface(surface);
        }
    }

    return world.ui.textCache[key] = cache;
}
