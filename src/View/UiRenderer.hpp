#pragma once

//============================

#include "../Model/World.hpp"
#include "TextRenderer.hpp"
#include "UiVisibility.hpp"
#include "../utils.hpp"
#include <algorithm>

//============================

#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>
#include <string>


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

inline void renderTooltip(World& world) {
    constexpr Uint32 kTooltipDelayMs = 1000;
    if (world.ui.hoveredTooltip.empty() ||
        SDL_GetTicks() - world.ui.hoverStartTicks < kTooltipDelayMs) {
        return;
    }

    TextCache& text = getOrRenderText(world, "country_medium", world.ui.hoveredTooltip);
    if (!text.texture) return;

    constexpr float kPadding = 8.0f;
    constexpr float kCursorOffset = 14.0f;
    constexpr float kScreenMargin = 4.0f;
    int screenW, screenH;
    SDL_GetWindowSize(world.window, &screenW, &screenH);

    SDL_FRect rect{
        static_cast<float>(world.ui.mouseX) + kCursorOffset,
        static_cast<float>(world.ui.mouseY) + kCursorOffset,
        static_cast<float>(text.w) + 2.0f * kPadding,
        static_cast<float>(text.h) + 2.0f * kPadding
    };
    if (rect.x + rect.w > screenW - kScreenMargin) {
        rect.x = static_cast<float>(screenW) - rect.w - kScreenMargin;
    }
    if (rect.y + rect.h > screenH - kScreenMargin) {
        rect.y = static_cast<float>(world.ui.mouseY) - rect.h - kScreenMargin;
    }
    rect.x = std::max(kScreenMargin, rect.x);
    rect.y = std::max(kScreenMargin, rect.y);

    Uint8 previousR, previousG, previousB, previousA;
    SDL_GetRenderDrawColor(world.renderer, &previousR, &previousG, &previousB, &previousA);
    SDL_SetRenderDrawColor(world.renderer, 22, 18, 14, 240);
    SDL_RenderFillRectF(world.renderer, &rect);
    SDL_SetRenderDrawColor(world.renderer, 190, 155, 85, 255);
    SDL_RenderDrawRectF(world.renderer, &rect);

    SDL_FRect textRect{
        rect.x + kPadding,
        rect.y + kPadding,
        static_cast<float>(text.w),
        static_cast<float>(text.h)
    };

    SDL_RenderCopyF(world.renderer, text.texture, nullptr, &textRect);
    SDL_SetRenderDrawColor(world.renderer, previousR, previousG, previousB, previousA);
}

inline void renderUI(World& world) {
    for (auto& element : world.ui.uiElements) {
        if (!areUiConditionsMet(world, element)) continue;
        renderElement(world, element);
    }
    renderTooltip(world);
}