#include "Renderer.hpp"
#include "utils.hpp"
#include <algorithm>

void render(GameData& state, SDL_Renderer* renderer, SDL_Window* window) {
    // Obtener dimensiones de la ventana
    int winWidth, winHeight;
    SDL_GetWindowSize(window, &winWidth, &winHeight);
    
    // Calcular escala
    float baseScale = std::min(
        static_cast<float>(winWidth) / state.texWidth,
        static_cast<float>(winHeight) / state.texHeight
    );
    float finalScale = baseScale * state.scale;
    
    // Limpiar pantalla
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    
    // Rect de destino
    SDL_FRect destRect = {
        state.offsetX,
        state.offsetY,
        static_cast<float>(state.texWidth) * finalScale,
        static_cast<float>(state.texHeight) * finalScale
    };
    
    // Renderizar mapa base (Terreno)
    displayTexture(renderer, state.terrain, destRect, 255);
    // Renderizar mapa base (países)
    displaySurface(renderer, state.countries, destRect, 255);
    
    // Renderizar todas las fronteras (gris oscuro)
   // displayFrontiers(state, renderer, finalScale, SDL_Color{0, 0, 0, 100});   // esto lleva los fps de 600 a 11 no usar
    
    displayTexture(renderer, state.frontierTexture, destRect, 255);
    
    // Renderizar fronteras seleccionadas (amarillo)
    HighlightProvince(state, renderer, finalScale, SDL_Color{255, 255, 0, 240}, state.selectedProvince);
    
    // Presentar
    SDL_RenderPresent(renderer);
}

/*
void displayFrontiers(GameData& state, SDL_Renderer* renderer, float finalScale, SDL_Color color) {   // renderiza en cada frame un punto por cada punto de frontera
                                                                                                        
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    for (const auto& [colorPair, points] : state.frontierList) {
        const auto& [colorA, colorB] = colorPair;
        
        for (const auto& point : points) {
            float sx = state.offsetX + point.x * finalScale;
            float sy = state.offsetY + point.y * finalScale;
            SDL_FRect dot = {sx, sy, finalScale, finalScale};
            SDL_RenderFillRectF(renderer, &dot);
        }
    }
}
    */

void HighlightProvince(GameData& state, SDL_Renderer* renderer, float finalScale, SDL_Color color, uint32_t provinceColor) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // search the province frontiers

    std::vector<const std::vector<SDL_Point>*> matchingLines;

    for (const auto& [colorPair, points] : state.frontierList) {
        const auto& [colorA, colorB] = colorPair;

        if (colorA == provinceColor || colorB == provinceColor) {
            matchingLines.push_back(&points);
        }
    }

    // paint them in a color

    for (const auto* points : matchingLines) {
        for (const auto& point : *points) {
            float sx = state.offsetX + point.x * finalScale;
            float sy = state.offsetY + point.y * finalScale;
            SDL_FRect dot = {sx, sy, finalScale, finalScale};
            SDL_RenderFillRectF(renderer, &dot);
        }
    }
}

void displaySurface(SDL_Renderer* renderer, SDL_Surface* surface, const SDL_FRect& destRect, Uint8 alpha) {
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) return;
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderCopyF(renderer, texture, nullptr, &destRect);
    SDL_DestroyTexture(texture);
}
void displayTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect& destRect, Uint8 alpha) {
    if (!texture) return;
    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderCopyF(renderer, texture, nullptr, &destRect);
}
