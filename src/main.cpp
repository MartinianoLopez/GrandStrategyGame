#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "Model/World.hpp"
#include "View/Renderer.hpp"
#include "Controller/gameWindow.hpp"
#include "Controller/debugWindow.hpp"
#include "Controller/eventRouter.hpp"
#include <SDL2/SDL_ttf.h>
#include "Model/Loader.hpp"
#include "View/UIManager.hpp"
#include "Simulation/Time.hpp"
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

    Uint32 lastTicks = SDL_GetTicks();

while (world.running) {
    Uint32 frameStart = SDL_GetTicks();
    float deltaTime = (frameStart - lastTicks) / 1000.0f;
    lastTicks = frameStart;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        eventManager.route(world, event, mainWin, debugWin);
        buildUI(world, mainWin.renderer);
    }

    if (world.timePaused == false)
        timeRun(world, deltaTime);

    renderGame(world, mainWin.renderer, mainWin.window);
    if (debugging) debugWin.render(world);

    Uint32 frameTime = SDL_GetTicks() - frameStart;
    world.fps = 1000.0f / (frameTime > 0 ? frameTime : 1);
    if (frameTime < (Uint32)world.frameDelay)
        SDL_Delay(world.frameDelay - frameTime);
}

    // =========================================================================================
    // Shutdown                                                                                        
    // =========================================================================================

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