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

inline void updateWindowVariables(World& world){
    SDL_Window* window = world.window;
    int winWidth, winHeight;
    SDL_GetWindowSize(window, &winWidth, &winHeight); 
    world.winWidth = winWidth;
    world.winHeight = winHeight;

    world.finalScale = std::min(
        (float)world.winWidth  / world.texWidth,
        (float)world.winHeight / world.texHeight
    ) * world.scale;

    world.destRect = {
        world.offsetX,
        world.offsetY,
        world.texWidth  * world.finalScale,
        world.texHeight * world.finalScale
    };
}

inline void renderGame(World& world) {
    updateWindowVariables(world);

    SDL_RenderClear(world.renderer);

    renderMapLayer( world);
    renderUI(world);  

    SDL_RenderPresent(world.renderer);
    
}
