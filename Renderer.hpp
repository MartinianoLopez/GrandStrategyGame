#pragma once
#include <SDL2/SDL.h>
#include "GameData.hpp"

void render(GameData& state, SDL_Renderer* renderer, SDL_Window* window);
void displayFrontiers(GameData& state, SDL_Renderer* renderer, float finalScale, SDL_Color color);
void HighlightProvince(GameData& state, SDL_Renderer* renderer, float finalScale, SDL_Color color, uint32_t provinceColor);
void displayTerrain(SDL_Renderer* renderer, SDL_Surface* surface, const SDL_FRect& destRect, Uint8 alpha);

