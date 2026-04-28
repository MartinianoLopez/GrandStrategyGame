#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "GameData.hpp"
#include "EventHandler.hpp"
#include "Renderer.hpp"
#include "Loader.hpp"
int main() {
std::cerr << "START\n";
SDL_Init(SDL_INIT_VIDEO);
IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG;
SDL_Window* window = SDL_CreateWindow( "Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
GameData state;
std::cerr << "LOADING...\n";
loadAssets(state);
std::cerr << "LOADED\n";
bool running = true;
while (running) {
Uint32 frameStart = SDL_GetTicks();
handleEvents(state, window, running);
render(state, renderer, window);
Uint32 frameTime = SDL_GetTicks() - frameStart;
if (frameTime < (Uint32)state.frameDelay)
SDL_Delay(state.frameDelay - frameTime);
    }
SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);
IMG_Quit();
SDL_Quit();
std::cerr << "END\n";
return 0;
}