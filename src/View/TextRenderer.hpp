#pragma once

//============================

#include "../Model/World.hpp"

//============================

#include "SDL_rect.h"
#include "SDL_render.h"
#include <string>

//============================

inline void renderText(
    SDL_Renderer* renderer,
    SDL_FRect rect,
    World& world,
    const std::string& fontId,
    int x,
    int y,
    const std::string& text
) {
    Font* font = nullptr;
    for (auto& f : world.fonts) {
        if (f.id == fontId) {
            font = &f;
            break;
        }
    }
    if (!font) return;

    for (char c : text) {
        unsigned char uc = static_cast<unsigned char>(c);

        if (uc >= 128)
            continue;

        Glyph& g = font->glyphs[(int)uc];

        if (!g.tex) continue;

        SDL_Rect dst = {
            x,
            y,
            g.w,
            g.h
        };

        SDL_RenderCopy(renderer, g.tex, nullptr, &dst);

        x += g.w;
    }
}