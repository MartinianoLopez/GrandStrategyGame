#pragma once

//============================

#include "../Model/World.hpp"
#include "TextRenderer.hpp"

//============================

#include "SDL_rect.h"
#include "SDL_render.h"
#include <algorithm>
#include <string>


inline void renderUI(SDL_Renderer* renderer, World& world, SDL_Window* window) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    for (auto& element : world.ui.uiElements) {
        SDL_FRect rect = element.calculateBase(w, h);

        if(element.name == "declareWarBtn" && world.selectedCountry == world.playerCountry){
            continue;
        }
        if(world.selectedCountry == "NONE" && (element.name == "countryFlagFrame" || element.name == "countryFlagTex" || element.name == "sidePanel"|| element.name == "declareWarBtn")){
            continue;
        }
        
        if (element.texture) {
            SDL_RenderCopyF(renderer, element.texture, nullptr, &rect);
        } else {
            SDL_SetRenderDrawColor(renderer, element.color.r, element.color.g, element.color.b, element.color.a);
            SDL_RenderFillRectF(renderer, &rect);
        }

        if (element.getText) {
            std::string text = element.getText();

            Font* font = nullptr;
            for (auto& f : world.fonts) {
                if (f.id == element.font) { font = &f; break; }
            }
            if (!font) continue;

            int tw = 0, th = 0;
            for (unsigned char c : text) {
                if (c >= 128) continue;
                Glyph& g = font->glyphs[c];
                tw += g.w;
                th = std::max(th, g.h);
            }

            int tx = (int)(rect.x + (rect.w - tw) * 0.5f);
            int ty = (int)(rect.y + (rect.h - th) * 0.5f);

            renderText(renderer, rect, world, element.font, tx, ty, text);
        }
    }
}
