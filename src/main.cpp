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
   
        UIRegistry reg;
        initRegistry(reg, world);
        

    std::cerr << "LOADED\n";
    
    // =========================================================================================
    // Game Loop                                                                                          
    // =========================================================================================

    Uint32 lastTicks    = SDL_GetTicks();
    Uint32 lastUIReload = SDL_GetTicks();
    MenuPlace lastPlace = world.place;
    loadUIFromFile(world, reg, "assets/ui/ui_layout.txt", mainWin.renderer);

    while (world.running) {

        // ── Timing ───────────────────────────────────────
        Uint32 frameStart = SDL_GetTicks();
        float deltaTime   = (frameStart - lastTicks) / 1000.0f;
        lastTicks         = frameStart;

        // ── Events ───────────────────────────────────────
        SDL_Event event;
        while (SDL_PollEvent(&event))
            eventManager.route(world, event, mainWin, debugWin);

        // ── Simulation ───────────────────────────────────
        if (!world.timePaused)
            timeRun(world, deltaTime);

        // ── UI ───────────────────────────────────────────
        bool placeChanged = world.place != lastPlace;
        bool timerFired   = (frameStart - lastUIReload) >= 2000;

        if (placeChanged || (debugging && timerFired)) {
            loadUIFromFile(world, reg, "assets/ui/ui_layout.txt", mainWin.renderer);
            lastPlace    = world.place;
            lastUIReload = frameStart;
        }

        // ── Render ───────────────────────────────────────
        renderGame(world, mainWin.renderer, mainWin.window);
        if (debugging) debugWin.render(world);

        // ── Frame cap ────────────────────────────────────
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        world.fps        = 1000.0f / (frameTime > 0 ? frameTime : 1);
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