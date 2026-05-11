#pragma once
#include <SDL2/SDL.h>
#include <algorithm>
#include "World.hpp"
#include "utils.hpp"

static void processEvent(World& world, SDL_Window* window, bool& running, const SDL_Event& event) {

    switch (event.type) {

        case SDL_QUIT:
            running = false;
            break;

        case SDL_MOUSEWHEEL: {
            float zoomFactor = (event.wheel.y > 0) ? 1.1f : 0.9f;

            float mx = (float)event.wheel.mouseX;
            float my = (float)event.wheel.mouseY;

            world.offsetX = mx + (world.offsetX - mx) * zoomFactor;
            world.offsetY = my + (world.offsetY - my) * zoomFactor;

            world.scale *= zoomFactor;
            break;
        }

        case SDL_MOUSEBUTTONDOWN: {

            // RIGHT CLICK
            if (event.button.button == SDL_BUTTON_RIGHT) {

                world.dragging = true;
                world.lastX = event.button.x;
                world.lastY = event.button.y;

                int winWidth, winHeight;
                SDL_GetWindowSize(window, &winWidth, &winHeight);

                float baseScale = std::min(
                    (float)winWidth / world.texWidth,
                    (float)winHeight / world.texHeight
                );

                float finalScale = baseScale * world.scale;

                int imgX = static_cast<int>(
                    (event.button.x - world.offsetX) / finalScale
                );

                int imgY = static_cast<int>(
                    (event.button.y - world.offsetY) / finalScale
                );

              if(imgX >= 0 && imgX < world.texWidth &&
                  imgY >= 0 && imgY < world.texHeight) {

                  uint32_t targetColor =
                      getPixelColor(world.provincesBmp, imgX, imgY);

                  Province* province =
                      provinceFindByColor(world.provinces, targetColor);

                  if (province == nullptr)
                      return;

                  world.objectiveProvince = province->id;

                  if (world.objectiveProvince == world.selectedProvince)
                      return;

                  Army* army =
                      armyPositionFind(
                          world.armies,
                          world.selectedProvince
                      );

                  if (army == nullptr)
                      return;

                  army->position = world.objectiveProvince;
              }
            }

            // LEFT CLICK
            if (event.button.button == SDL_BUTTON_LEFT) {

                int winWidth, winHeight;

                SDL_GetWindowSize(window, &winWidth, &winHeight);

                float baseScale = std::min(
                    (float)winWidth / world.texWidth,
                    (float)winHeight / world.texHeight
                );

                float finalScale = baseScale * world.scale;

                float mx = (float)event.button.x;
                float my = (float)event.button.y;

                int imgX = static_cast<int>(
                    (mx - world.offsetX) / finalScale
                );

                int imgY = static_cast<int>(
                    (my - world.offsetY) / finalScale
                );

                if (imgX >= 0 && imgX < world.texWidth &&
                    imgY >= 0 && imgY < world.texHeight) {

                    uint32_t provinceColor =
                        getPixelColor(world.provincesBmp, imgX, imgY);

                    auto it =
                        provinceFindByColor(world.provinces, provinceColor);

                    if (it == nullptr) {
                        std::cerr << "Color not found: "
                                  << colorToString(provinceColor)
                                  << "\n";
                        break;
                    }

                    world.selectedProvince = it->id;
                }
            }

            break;
        }

        case SDL_MOUSEBUTTONUP:

            if (event.button.button == SDL_BUTTON_RIGHT) {
                world.dragging = false;
            }

            break;

        case SDL_MOUSEMOTION:

            if (world.dragging) {

                world.offsetX += event.motion.x - world.lastX;
                world.offsetY += event.motion.y - world.lastY;

                world.lastX = event.motion.x;
                world.lastY = event.motion.y;
            }

            break;
    }
}

void handleEvents(World& world, SDL_Window* window, bool& running) {

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        processEvent(world, window, running, event);
    }
}

void handleEvent(World& world, SDL_Window* window, bool& running, const SDL_Event& event) {
    processEvent(world, window, running, event);
}