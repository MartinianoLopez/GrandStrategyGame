#pragma once

//=============================

#include "UiRenderer.hpp"
#include "MapRenderer.hpp"
#include "../Model/World.hpp"

//=============================

#include "SDL_render.h"

//=============================

inline void renderMapLayer(World& world, SDL_Renderer* renderer, SDL_Window* window){

    renderMap(world, renderer, window, false); 
    renderMap(world, renderer, window, true);

}

inline void renderGame(World& world, SDL_Renderer* renderer, SDL_Window* window) {

    SDL_RenderClear(renderer);

    renderMapLayer( world, renderer, window);
    renderUI(renderer, world, window);  

    SDL_RenderPresent(renderer);
    
}
