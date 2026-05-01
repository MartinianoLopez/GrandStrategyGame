#pragma once
#include <SDL2/SDL.h>
#include "GameData.hpp"
#include "debugWindow.hpp"
#include "EventHandler.hpp"

struct EventManager {
    void process(SDL_Event& event, GameData& state, SDL_Window* window,
                 DebugWindow& debugWin, bool& running) {
        Uint32 mainWinID  = SDL_GetWindowID(window);
        Uint32 eventWinID = 0;

        if      (event.type == SDL_MOUSEMOTION)                        eventWinID = event.motion.windowID;
        else if (event.type == SDL_MOUSEBUTTONDOWN ||
                 event.type == SDL_MOUSEBUTTONUP)                      eventWinID = event.button.windowID;
        else if (event.type == SDL_MOUSEWHEEL)                         eventWinID = event.wheel.windowID;
        else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) eventWinID = event.key.windowID;
        else if (event.type == SDL_WINDOWEVENT)                        eventWinID = event.window.windowID;

        if (event.type == SDL_QUIT ||
           (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)) {
            running = false;
            return;
        }

        if (debugWin.window) {
            Uint32 debugWinID = SDL_GetWindowID(debugWin.window);
            if (eventWinID == debugWinID || eventWinID == 0) debugWin.processEvent(event);
        }

        if (eventWinID == mainWinID || eventWinID == 0) handleEvent(state, window, running, event);
    }
};