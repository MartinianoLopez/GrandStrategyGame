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
    state.finalScale = baseScale * state.scale;
    // Limpiar pantalla
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    
    // Rect de destino
    SDL_FRect destRect = {
        state.offsetX,
        state.offsetY,
        static_cast<float>(state.texWidth) * state.finalScale,
        static_cast<float>(state.texHeight) * state.finalScale
    };
    displayTexture(renderer, state.height, destRect, 255);
    // Renderizar mapa base (Terreno)
    displayTexture(renderer, state.terrain, destRect, 150);
    // Renderizar mapa base (países)
    displaySurface(renderer, state.countries, destRect, 240);
    
    // Renderizar todas las fronteras (gris oscuro)
    // displayFrontiers(state, renderer, finalScale, SDL_Color{0, 0, 0, 100});   // esto lleva los fps de 600 a 11 no usar
    
    displayTexture(renderer, state.frontierTexture, destRect, 200);
    
    // Renderizar fronteras seleccionadas (amarillo)
    HighlightProvince(state, renderer, state.finalScale, SDL_Color{255, 255, 0, 240}, state.selectedProvince);

    renderProvinceIds(renderer, state.digits, state.finalScale, state, winWidth, winHeight);
    
    // displayPoints(state, renderer, state.finalScale, state.ProvincesCenterList, SDL_Color{255, 255, 0, 240});
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

void displayPoints(GameData& state, SDL_Renderer* renderer, float finalScale, 
                   const std::map<uint32_t, SDL_Point>& points, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (const auto& [id, point] : points) {
        float sx = state.offsetX + point.x * finalScale;
        float sy = state.offsetY + point.y * finalScale;
        SDL_FRect dot = {sx, sy, finalScale * 3, finalScale * 3}; // 3x bigger to be visible
        SDL_RenderFillRectF(renderer, &dot);
    }
}

void renderProvinceIds(SDL_Renderer* renderer, SDL_Texture** digits, float finalScale, const GameData& state, int screenW, int screenH) {
    if (finalScale <= 2.5f) return;
    for (const auto& [color, center] : state.ProvincesCenterList) {
        float sx = state.offsetX + center.x * finalScale;
        float sy = state.offsetY + center.y * finalScale;
        if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue; // fuera de pantalla
        if (!state.BmpColorToProvinceId.count(color)) continue;
        uint32_t id = state.BmpColorToProvinceId.at(color);
        renderNumber(renderer, digits, (int)sx, (int)sy, id);
    }
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, int x, int y, 
                const std::string& text, SDL_Color color) {
    SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    SDL_Rect dst = {x, y, 0, 0};
    SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}



// Render de cualquier número
void renderNumber(SDL_Renderer* r, SDL_Texture** digits, int x, int y, int number) {
    std::string s = std::to_string(number);
    int charW = 10;
    int totalW = s.size() * charW;
    x -= totalW / 2;  // centrado horizontal
    y -= 8;           // centrado vertical (mitad de 16)
    for (char c : s) {
        SDL_Rect dst = {x, y, charW, 16};
        SDL_RenderCopy(r, digits[c - '0'], NULL, &dst);
        x += charW;
    }
}