#include "utils.hpp"
#include <algorithm>

void renderFrontiers(GameData& state, SDL_Renderer* renderer, SDL_Color color, int screenW, int screenH) {
                                                                                                        
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (const auto& [colorPair, points] : state.frontierList) {
        const auto& [colorA, colorB] = colorPair;
        
        for (const auto& point : points) {
            float sx = state.offsetX + point.x * state.finalScale;
            float sy = state.offsetY + point.y * state.finalScale;
            if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue;
            SDL_FRect dot = {sx, sy, state.finalScale, state.finalScale};
            SDL_RenderFillRectF(renderer, &dot);
        }
    }
}

void markProvinceFrontiers(GameData& state, SDL_Renderer* renderer, SDL_Color remarkColor, uint32_t provinceColor) {
    SDL_SetRenderDrawColor(renderer, remarkColor.r, remarkColor.g, remarkColor.b, remarkColor.a);

    std::vector<const std::vector<SDL_FPoint>*> matchingLines;
    //Search
    for (const auto& [colorPair, points] : state.frontierList) {
        const auto& [colorA, colorB] = colorPair;

        if (colorA == provinceColor || colorB == provinceColor) {
            matchingLines.push_back(&points);
        }
    }
    //Render
    for (const auto* points : matchingLines) {
        for (const auto& point : *points) {
            float sx = state.offsetX + point.x * state.finalScale;
            float sy = state.offsetY + point.y * state.finalScale;
            SDL_FRect dot = {sx, sy, state.finalScale, state.finalScale};
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

void renderNumber(SDL_Renderer* r, GameData& state, int x, int y, int number, bool selected) {
    std::string s = std::to_string(number);
    int charW = 10;
    int totalW = s.size() * charW;
    int padding = 3;
    x -= totalW / 2;
    y -= 8;

    SDL_Rect bg = {x - padding, y - padding, totalW + padding*2, 16 + padding*2};
    SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
    SDL_RenderFillRect(r, &bg);

    if (selected) {
        SDL_Rect border = {bg.x - 2, bg.y - 2, bg.w + 4, bg.h + 4};
        SDL_SetRenderDrawColor(r, 255, 255, 0, 255);
        SDL_RenderDrawRect(r, &border);
    }

    for (char c : s) {
        SDL_Rect dst = {x, y, charW, 16};
        SDL_RenderCopy(r, state.digits[c - '0'], NULL, &dst);
        x += charW;
    }
}

void renderProvinceIds(SDL_Renderer* renderer, float finalScale, GameData& state, int screenW, int screenH) {
    if (finalScale <= 2.5f) return;
    for (const auto& [color, center] : state.ProvincesCenterList) {
        float sx = state.offsetX + center.x * finalScale;
        float sy = state.offsetY + center.y * finalScale;
        if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue; // fuera de pantalla
        if (!state.BmpColorToProvinceId.count(color)) continue;
        uint32_t id = state.BmpColorToProvinceId.at(color);
        renderNumber(renderer, state, (int)sx, (int)sy, id, false);
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




void renderTroops(GameData& state, SDL_Renderer* renderer, int screenW, int screenH) {

    for (const auto& [color, center] : state.ProvincesCenterList) {
        float sx = state.offsetX + center.x * state.finalScale;
        float sy = state.offsetY + center.y * state.finalScale;
        if (sx < 0 || sy < 0 || sx > screenW || sy > screenH) continue;
        if (!state.BmpColorToProvinceId.count(color)) continue;
        uint32_t id = state.BmpColorToProvinceId.at(color);
        if (!state.troopsList.count(id)) continue; // provincia sin tropas, no mostrar
        if (state.selectedProvince == color) {
            renderNumber(renderer, state, (int)sx, (int)sy, state.troopsList.at(id), true);
        } else {
            renderNumber(renderer, state, (int)sx, (int)sy, state.troopsList.at(id), false);
        }
    }
}

void render(GameData& state, SDL_Renderer* renderer, SDL_Window* window) {
    // Obtener dimensiones de la ventana
    
    int winWidth, winHeight;
    SDL_GetWindowSize(window, &winWidth, &winHeight);
    state.finalScale = std::min((float)winWidth / state.texWidth, (float)winHeight / state.texHeight) * state.scale;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    SDL_FRect destRect = {state.offsetX, state.offsetY, state.texWidth * state.finalScale, state.texHeight * state.finalScale};


    displayTexture(renderer, state.height,    destRect, 255);

    displayTexture(renderer, state.terrain,   destRect, 150);

    displaySurface(renderer, state.countries, destRect, 240);
    
     if (state.scale > 5.0f){

        renderFrontiers(state, renderer,       SDL_Color{0, 0, 0, 150},        winWidth, winHeight); 
    
        markProvinceFrontiers(state, renderer, SDL_Color{255, 255, 0, 240}, state.selectedProvince);

        renderTroops(state, renderer, winWidth, winHeight);

    }

    // displayPoints(state, renderer, state.finalScale, state.ProvincesCenterList, SDL_Color{255, 255, 0, 240});

    // if(state.finalScale > 1){
    //   displayTexture(renderer, state.frontierTexture, destRect, 100);
    // }


    SDL_RenderPresent(renderer);
}