#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>

#include "GameData.hpp"
#include "Loader.hpp"
#include "Renderer.hpp"
#include "EventManager.h"
#include "debugWindow.h"

int main() {
  // =========================================================================================
  // Init
  // =========================================================================================
    std::cerr << "START\n";

    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)               return -1;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))    return -1;

    window = SDL_CreateWindow(
        "Window",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) return -1;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return -1;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);


    // =========================================================================================
    // Debug window
    // =========================================================================================
    bool debugging = false;

    DebugWindow debugWin;
    if (debugging) debugWin.init();

    // =========================================================================================
    // Load game
    // =========================================================================================
    GameData state;
    std::cerr << "LOADING...\n";
    loadAssets(state, renderer);
    std::cerr << "LOADED\n";

    EventManager eventManager;

    // =========================================================================================
    // Game Loop                                                                                          
    // =========================================================================================
    bool running = true;

    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        // Events
        SDL_Event event;
        while (SDL_PollEvent(&event))
            eventManager.process(event, state, window, debugWin, running);

        // Update & render
        render(state, renderer, window);
        if (debugging) debugWin.render(state);

        // Frame cap
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        state.fps = 1000.0f / (frameTime > 0 ? frameTime : 1);
        if (frameTime < (Uint32)state.frameDelay)
        SDL_Delay(state.frameDelay - frameTime);   
        std::cerr << "FPS: " << (int)state.fps << "\r";              
    }

    // =========================================================================================
    // Shutdown                                                                                        
    // =========================================================================================
    if (debugging) debugWin.shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    std::cerr << "END\n";
    return 0;
}