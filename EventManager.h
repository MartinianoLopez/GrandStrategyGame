#pragma once
#include <SDL2/SDL.h>
#include "GameData.hpp"
#include "debugWindow.h"

class EventManager {
public:
    void process(SDL_Event& event, GameData& state, SDL_Window* window,
                 DebugWindow& debugWin, bool& running);
};