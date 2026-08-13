#pragma once

//=============================

#include "UiRenderer.hpp"
#include "MapRenderer.hpp"
#include "../Model/World.hpp"

//=============================

#include "SDL_render.h"

//=============================

inline void renderMapLayer(World& world){

    renderMap(world, false); 
    renderMap(world, true);

}

inline void renderGame(World& world) {

    SDL_RenderClear(world.renderer);

    renderMapLayer( world);
    renderUI(world);  

    SDL_RenderPresent(world.renderer);
    
}
