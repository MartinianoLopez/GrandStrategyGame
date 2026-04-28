#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "GameData.hpp"
#include "EventHandler.hpp"
#include "Renderer.hpp"
#include "Loader.hpp"
#include "debugWindow.h"

int main() {
  std::cerr << "START\n";
  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG;

  SDL_Window* window = SDL_CreateWindow(
    "Window",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    800, 600,
    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
  );
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  GameData state;

  DebugWindow debugWin;
  debugWin.init();

  std::cerr << "LOADING...\n";
  loadAssets(state);
  std::cerr << "LOADED\n";

  bool running = true;
  while (running) {
    Uint32 frameStart = SDL_GetTicks();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      Uint32 mainWinID  = SDL_GetWindowID(window);
      Uint32 debugWinID = SDL_GetWindowID(debugWin.window);
      Uint32 eventWinID = 0;

      if (event.type == SDL_MOUSEMOTION)                eventWinID = event.motion.windowID;
      else if (event.type == SDL_MOUSEBUTTONDOWN ||
              event.type == SDL_MOUSEBUTTONUP)          eventWinID = event.button.windowID;
      else if (event.type == SDL_MOUSEWHEEL)             eventWinID = event.wheel.windowID;
      else if (event.type == SDL_KEYDOWN ||
              event.type == SDL_KEYUP)                  eventWinID = event.key.windowID;
      else if (event.type == SDL_WINDOWEVENT)            eventWinID = event.window.windowID;

      // cerrar cualquier ventana cierra la app
      if (event.type == SDL_QUIT) {
        running = false;
        break;
      }
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE) {
        running = false;
        break;
      }

      if (eventWinID == debugWinID || eventWinID == 0) {
        debugWin.processEvent(event);
      }
      if (eventWinID == mainWinID || eventWinID == 0) {
        handleEvent(state, window, running, event);
      }
    }

    render(state, renderer, window);
    debugWin.render(state);

    Uint32 frameTime = SDL_GetTicks() - frameStart;
    if (frameTime < (Uint32)state.frameDelay)
      SDL_Delay(state.frameDelay - frameTime);
  }

  debugWin.shutdown();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
  std::cerr << "END\n";
  return 0;
}