#pragma once
#include <SDL2/SDL.h>
#include "World.hpp"
#include "debugWindow.hpp"
#include "EventHandler.hpp"

struct EventManager {
    void process(SDL_Event& event, World& world, SDL_Window* window, DebugWindow& debugWin, bool& running) {
        
        // quit
        if (event.type == SDL_QUIT ||
           (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)) {
            running = false;
            return;
        }

        // get event window id
        Uint32 mainWinID  = SDL_GetWindowID(window);
        Uint32 eventWinID = 0;
        switch (event.type) {
            case SDL_MOUSEMOTION:     eventWinID = event.motion.windowID; break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:   eventWinID = event.button.windowID; break;
            case SDL_MOUSEWHEEL:      eventWinID = event.wheel.windowID;  break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:           eventWinID = event.key.windowID;    break;
            case SDL_WINDOWEVENT:     eventWinID = event.window.windowID; break;
        }

        // dispatch
        if (debugWin.window) {
            Uint32 debugWinID = SDL_GetWindowID(debugWin.window);
            if (eventWinID == debugWinID || eventWinID == 0)
                debugWin.processEvent(event);
        }

        if (eventWinID == mainWinID || eventWinID == 0)
            handleEvent(world, window, running, event);
    }
};