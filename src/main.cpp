// ================================

#include "Model/World.hpp"
#include "View/Renderer.hpp"
#include "Controller/InputHandler.hpp"
#include "Model/Loader.hpp"
#include "Model/UiLoader.hpp"
#include "Simulation/Time.hpp"

//================================

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

//=============================================================

void loadGame(World& world){

    { Timer t("TotalAssets"); loadAssets(world); }
    { Timer t("Ui");          initUi(world); }
    { Timer t("UiLayout");    parseLayout(world); }

    world.lastTicks    = SDL_GetTicks();
    world.lastUIReload = SDL_GetTicks();
    world.lastPlace    = world.ui.place;
}

//=============================================================

void update(World& world) {

    Uint32 frameStart = SDL_GetTicks();
    float deltaTime   = (frameStart - world.lastTicks) / 1000.0f;
    world.lastTicks   = frameStart;

    SDL_Event event;

    while (SDL_PollEvent(&event)){
        processEvent(world, event);
    }

    updateTime(world, deltaTime);

    reloadUI(world, frameStart);
    reloadFlagTextures(world);

    renderGame(world);

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

//=============================================================

void runLoop(World& world) {
    #ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg([](void* arg){ main_loop(*(World*)arg); }, &world, 0, 1);
    #else
        while (world.running) update(world);
    #endif
}

//=============================================================

void shutdown(World& world){
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(world.renderer);
    SDL_DestroyWindow(world.window);
    SDL_Quit();
}

//=============================================================

int main() {

    World world = World();

    std::cerr << " -------- LOADING -------- \n";

    loadGame(world);

    std::cerr << " -------- RUNNING -------- \n";

    runLoop(world);

    std::cerr << " -------- TURNOFF -------- \n";

    shutdown(world);

    return 0;
}

