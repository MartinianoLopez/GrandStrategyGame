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

    for (auto& el : world.uiElements) {
        SDL_FRect rect = el.calculateBase(w, h);

        if(el.name == "declareWarBtn" && world.selectedCountry == world.playerCountry){
            continue;
        }
        if(world.selectedCountry == "NONE" && (el.name == "countryFlagFrame" || el.name == "countryFlagTex" || el.name == "sidePanel"|| el.name == "declareWarBtn")){
            continue;
        }
        
        if (el.texture) {
            SDL_RenderCopyF(renderer, el.texture, nullptr, &rect);
        } else {
            SDL_SetRenderDrawColor(renderer, el.color.r, el.color.g, el.color.b, el.color.a);
            SDL_RenderFillRectF(renderer, &rect);
        }

        if (el.getText) {
            std::string text = el.getText();

            Font* font = nullptr;
            for (auto& f : world.fonts) {
                if (f.id == el.font) { font = &f; break; }
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

            renderText(renderer, rect, world, el.font, tx, ty, text);
        }
    }
}
