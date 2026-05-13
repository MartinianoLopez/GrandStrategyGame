#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "World.hpp"
#include "Renderer.hpp"
#include "gameWindow.hpp"
#include "debugWindow.hpp"
#include "eventRouter.hpp"
#include <SDL2/SDL_ttf.h>
#include "Loader.hpp"

int main() {
    std::cerr << "START\n";

        GameWindow mainWin = GameWindow();
        DebugWindow debugWin;
        EventRouter eventManager;

        bool debugging = true;
        debugWin.init();
        if (!debugging) debugWin.hide();

    // =========================================================================================
    // LOADING                                                                                          
    // =========================================================================================
    
    std::cerr << "LOADING...\n";

        World world;

        loadAssets(world, mainWin.renderer);

    std::cerr << "LOADED\n";
        
        
    // =========================================================================================
    // Game Loop                                                                                          
    // =========================================================================================

    while (world.running) {
        Uint32 frameStart = SDL_GetTicks();

        // Events
        SDL_Event event;
        while (SDL_PollEvent(&event))
            eventManager.route(world, event, mainWin, debugWin);

        // Update & render
        renderDouble(world, mainWin.renderer, mainWin.window);
        if (debugging) debugWin.render(world);

        // Frame cap
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        world.fps = 1000.0f / (frameTime > 0 ? frameTime : 1);
        if (frameTime < (Uint32)world.frameDelay)
        SDL_Delay(world.frameDelay - frameTime);            
    }

    // =========================================================================================
    // Shutdown                                                                                        
    // =========================================================================================

    TTF_CloseFont(world.font);
    TTF_Quit();                
    SDL_Quit();
    if (debugging) debugWin.shutdown();
    SDL_DestroyRenderer(mainWin.renderer);
    SDL_DestroyWindow(mainWin.window);
    IMG_Quit();
    SDL_Quit();

    std::cerr << "END\n";
    return 0;
}