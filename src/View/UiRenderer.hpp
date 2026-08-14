#pragma once

//============================

#include "../Model/World.hpp"
#include "TextRenderer.hpp"
#include "../utils.hpp"

//============================

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_set>
#include <functional>
#include <algorithm>


//===========================

inline bool isElementHidden(World& world, const UIElement& element) {
    if (element.name == "declareWarBtn" && world.selectedCountry == world.playerCountry)
        return true;

    static const std::unordered_set<std::string> countryOnlyElements = {
        "countryFlagFrame", "countryFlagTex", "sidePanel", "declareWarBtn"
    };
    if (world.selectedCountry == "NONE" && countryOnlyElements.count(element.name))
        return true;

    return false;
}

inline void renderElementTexture(SDL_Renderer* renderer, const UIElement& element, const SDL_FRect& rect) {
    constexpr SDL_Color kMissingTextureColor = {255, 0, 255, 255}; // debug: texture failed to load

    if (element.texture) {
        SDL_RenderCopyF(renderer, element.texture, nullptr, &rect);
    } else {
        SDL_SetRenderDrawColor(renderer, kMissingTextureColor.r, kMissingTextureColor.g, kMissingTextureColor.b, kMissingTextureColor.a);
        SDL_RenderFillRectF(renderer, &rect);
    }
}

inline void renderElementText(World& world, const UIElement& element, const SDL_FRect& rect) {
    if (!element.textProvider) return;

    Font* font = nullptr;
    for (auto& f : world.fonts) {
        if (f.id == element.font) { font = &f; break; }
    }
    if (!font) return;

    std::string text = element.textProvider();

    int tw = 0, th = 0;
    for (unsigned char c : text) {
        if (c >= 128) continue;
        Glyph& g = font->glyphs[c];
        tw += g.w;
        th = std::max(th, g.h);
    }

    int tx = (int)(rect.x + (rect.w - tw) * 0.5f);
    int ty = (int)(rect.y + (rect.h - th) * 0.5f);

    renderText(rect, world, element.font, tx, ty, text);
}


inline void renderUI(World& world) {
    int w, h;
    SDL_GetWindowSize(world.window, &w, &h);

    for (auto& element : world.ui.uiElements) {
        if (isElementHidden(world, element)) continue;

        SDL_FRect rect = calculateBase(element, w, h);
        renderElementTexture(world.renderer, element, rect);
        renderElementText(world, element, rect);
    }
}