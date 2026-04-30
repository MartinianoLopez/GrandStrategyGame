#pragma once
#include <SDL2/SDL.h>
#include "GameData.hpp"
#include <SDL2/SDL_ttf.h>

void initialRender(GameData& state, SDL_Renderer* renderer, SDL_Window* window);
void render(GameData& state, SDL_Renderer* renderer, SDL_Window* window);
void displayFrontiers(GameData& state, SDL_Renderer* renderer, float finalScale, SDL_Color color);
void HighlightProvince(GameData& state, SDL_Renderer* renderer, float finalScale, SDL_Color color, uint32_t provinceColor);
void displaySurface(SDL_Renderer* renderer, SDL_Surface* surface, const SDL_FRect& destRect, Uint8 alpha);
void displayTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect& destRect, Uint8 alpha);
void displayPoints(GameData& state, SDL_Renderer* renderer, float finalScale, const std::map<uint32_t, SDL_Point>& points, SDL_Color color);
void renderProvinceIds(SDL_Renderer* renderer, SDL_Texture** digits, float finalScale, const GameData& state, int screenW, int screenH);
void renderText(SDL_Renderer* renderer, SDL_Texture* digits, int x, int y, const std::string& text, SDL_Color color = {255,255,255,255});
void renderNumber(SDL_Renderer* r, SDL_Texture** digits, int x, int y, int number, bool selected);
void renderTroops(SDL_Renderer* renderer, SDL_Texture** digits, float finalScale, const GameData& state, int screenW, int screenH );