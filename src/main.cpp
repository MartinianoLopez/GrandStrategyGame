#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

#include "Model/World.hpp"
#include "View/Renderer.hpp"
#include "Controller/gameWindow.hpp"
#include "Model/Loader.hpp"
#include "Model/UiLoader.hpp"
#include "Simulation/Time.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

GameWindow*  mainWinPtr  = nullptr;
World*       worldPtr    = nullptr;
bool debugging = true;

Uint32 lastTicks    = 0;
Uint32 lastUIReload = 0;
MenuPlace lastPlace;

void main_loop() {
    World&       world      = *worldPtr;
    GameWindow&  mainWin    = *mainWinPtr;

    Uint32 frameStart = SDL_GetTicks();
    float deltaTime   = (frameStart - lastTicks) / 1000.0f;
    lastTicks         = frameStart;

    SDL_Event event;
    while (SDL_PollEvent(&event))
        mainWin.processEvent(world, event);

    tick(world, deltaTime);

    bool placeChanged = world.place != lastPlace;
    bool timerFired   = (frameStart - lastUIReload) >= 2000;

    if (placeChanged || (debugging && timerFired)) {
        loadUIFromFile(world, "assets/ui/ui_layout.txt", mainWin.renderer);
        lastPlace    = world.place;
        lastUIReload = frameStart;
    }
    reloadFlagTextures(world);
    renderGame(world, mainWin.renderer, mainWin.window);

    Uint32 frameTime = SDL_GetTicks() - frameStart;
    world.fps = 1000.0f / (frameTime > 0 ? frameTime : 1);

#ifndef __EMSCRIPTEN__
    if (frameTime < (Uint32)world.frameDelay)
        SDL_Delay(world.frameDelay - frameTime);
#endif

#ifdef __EMSCRIPTEN__
    if (!world.running)
        emscripten_cancel_main_loop();
#endif
}

int main() {
    GameWindow mainWin = GameWindow();
    mainWinPtr = &mainWin;

    std::cerr << "LOADING...\n";

    World world = World(mainWin.renderer);
    worldPtr = &world;
    loadAssets(world, mainWin.renderer);

    initRegistry(world);
    loadUIFromFile(world, "assets/ui/ui_layout.txt", mainWin.renderer);

    lastTicks    = SDL_GetTicks();
    lastUIReload = SDL_GetTicks();
    lastPlace    = world.place;

    std::cerr << "START\n";

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (world.running) {
        main_loop();
    }
#endif

    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(mainWin.renderer);
    SDL_DestroyWindow(mainWin.window);
    SDL_Quit();

    std::cerr << "END\n";
    return 0;
}