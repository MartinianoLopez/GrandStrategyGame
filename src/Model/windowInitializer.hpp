#include "World.hpp"

inline void WindowInit(World &world){
            world.window = SDL_CreateWindow(
            "Window", 
            SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, 
            1920, 1080, 
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );
        world.renderer = SDL_CreateRenderer(
            world.window, 
            -1, 
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );
        SDL_SetRenderDrawBlendMode(world.renderer, SDL_BLENDMODE_BLEND);
}
