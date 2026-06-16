#pragma once
#include <SDL2/SDL.h>
#include "../Model/World.hpp"
#include "debugWindow.hpp"
#include "gameWindow.hpp"
/*
Acts over debug window and game window to determine on wich one the input event has ocurred

*/
struct EventRouter {


    void route(World& world, SDL_Event& event, GameWindow& mainWin, DebugWindow& debugWin) {
    Uint32 eventWinID = getEventWindowID(event);

    bool isClose = event.type == SDL_WINDOWEVENT && 
                   event.window.event == SDL_WINDOWEVENT_CLOSE;

    if (event.type == SDL_QUIT || (isClose && eventWinID == SDL_GetWindowID(mainWin.window))) {

        world.running = false;
        return;
    }

    if (isClose && debugWin.window && eventWinID == SDL_GetWindowID(debugWin.window)) {
        debugWin.hide();
        return;
    }

    if (mainWin.window && eventWinID == SDL_GetWindowID(mainWin.window))
        mainWin.processEvent(world, event);

    if (debugWin.window && eventWinID == SDL_GetWindowID(debugWin.window))
        debugWin.processEvent(event);
}


private:

    Uint32 getEventWindowID(const SDL_Event& event) {
        switch (event.type) {
            case SDL_MOUSEMOTION:                return event.motion.windowID;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:              return event.button.windowID;
            case SDL_MOUSEWHEEL:                 return event.wheel.windowID;
            case SDL_KEYDOWN:
            case SDL_KEYUP:                      return event.key.windowID;
            case SDL_WINDOWEVENT:                return event.window.windowID;
            default:                             return 0;
        }
    }
};