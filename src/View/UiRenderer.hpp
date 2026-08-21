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


//===========================
// this is a bad aproach
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

inline void renderElementTexture(World& world, const UIElement& element, const SDL_FRect& rect) {
    SDL_Renderer* renderer = world.renderer;
    constexpr SDL_Color kMissingTextureColor = {255, 0, 255, 255}; // debug: texture failed to load

    if (element.texture) {
        SDL_RenderCopyF(renderer, element.texture, nullptr, &rect);
    } else {
        SDL_SetRenderDrawColor(renderer, kMissingTextureColor.r, kMissingTextureColor.g, kMissingTextureColor.b, kMissingTextureColor.a);
        SDL_RenderFillRectF(renderer, &rect);
    }
}

inline void renderElementTextureHovered(World& world, const UIElement& element, const SDL_FRect& rect) {
    SDL_Renderer* renderer = world.renderer;

    if (element.texture) {
        SDL_SetTextureColorMod(element.texture, 180, 180, 180);
        SDL_RenderCopyF(renderer, element.texture, nullptr, &rect);
        SDL_SetTextureColorMod(element.texture, 255, 255, 255);
    } else {
        SDL_RenderFillRectF(renderer, &rect);
    }
}

inline void renderElementTexturePressed(World& world, const UIElement& element, const SDL_FRect& rect) {
    SDL_Renderer* renderer = world.renderer;

    if (element.texture) {
        SDL_SetTextureColorMod(element.texture, 180, 180, 180);
        SDL_RenderCopyF(renderer, element.texture, nullptr, &rect);
        SDL_SetTextureColorMod(element.texture, 255, 255, 255);
    } else {
        SDL_RenderFillRectF(renderer, &rect);
    }
}

inline void renderRects(SDL_Renderer* renderer, const SDL_FRect& rect) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRectF(renderer, &rect);
}

inline void renderElement(World& world, UIElement element){
    int w, h;
    SDL_GetWindowSize(world.window, &w, &h);
    SDL_FRect rect = calculateBase(element, w, h);
    
    if(element.name == world.ui.hoveredElement){
        renderElementTextureHovered(world, element, rect);
    }else if (world.ui.pressedElements.count(element.name)) {
        renderElementTexturePressed(world, element, rect);
    }else{
        renderElementTexture(world, element, rect);
    }
    renderElementText(world, element, rect);

    if (world.DEBUGGING_MODE) {
        renderRects(world.renderer, rect);
    }
}

inline void renderUI(World& world) {
    for (auto& element : world.ui.uiElements) {
        if (isElementHidden(world, element)) continue;
        renderElement(world, element);
    }
}