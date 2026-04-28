#pragma once
#include <SDL2/SDL.h>
#include "GameData.hpp"

void handleEvents(GameData& state, SDL_Window* window, bool& running);
void handleEvent(GameData& state, SDL_Window* window, bool& running, const SDL_Event& event);