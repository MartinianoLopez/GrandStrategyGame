#pragma once
#include <SDL2/SDL.h>
#include <algorithm>
#include "World.hpp"
#include "utils.hpp"
struct GameWindow {
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;

    GameWindow() {
        window   = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    bool tocaLimiteSuperior(World& world) {
        bool toca = world.offsetY >= 0;
        if (toca) SDL_Log("LIMITE SUPERIOR tocado | offsetY: %.2f", world.offsetY);
        return toca;
    }

    bool tocaLimiteInferior(World& world, int screenH) {
        float limitAbajo = screenH - world.texHeight * world.finalScale;
        bool toca = world.offsetY <= limitAbajo;
        if (toca) SDL_Log("LIMITE INFERIOR tocado | offsetY: %.2f | limitAbajo: %.2f", world.offsetY, limitAbajo);
        return toca;
    }

    bool tocaLimiteDerecha(World& world, int screenW) {
        float limitDerecha = screenW - world.texWidth * world.finalScale;
        bool toca = world.offsetX <= limitDerecha;
        if (toca) SDL_Log("LIMITE DERECHA tocado | offsetX: %.2f | limitDerecha: %.2f", world.offsetX, limitDerecha);
        return toca;
    }

    bool tocaLimiteIzquierda(World& world, int screenW) {
        float limitIzquierda = - screenW + world.texWidth * world.finalScale;
        bool toca = world.offsetX >= limitIzquierda;
        if (toca) SDL_Log("LIMITE IZQUIERDA tocado | offsetX: %.2f | limitIzquierda: %.2f", world.offsetX, limitIzquierda);
        return toca;
    }

    void processEvent(World& world, const SDL_Event& event) {

        switch (event.type) {

            //==================================================================================================================
            // zoom
            //==================================================================================================================
                
            case SDL_MOUSEWHEEL: {

                float zoomFactor = (event.wheel.y > 0) ? 1.1f : 0.9f;
                float mx = (float)event.wheel.mouseX;
                float my = (float)event.wheel.mouseY;

                if (world.scale * zoomFactor <= 1.5) {
                    break;
                }
                    world.offsetX = mx + (world.offsetX - mx) * zoomFactor; // real offset offsetX / zoomfactor
                    world.offsetY = my + (world.offsetY - my) * zoomFactor;
                    world.scale *= zoomFactor;
                        
                    int screenW, screenH;
                    SDL_GetWindowSize(window, &screenW, &screenH);

                    world.finalScale = std::min(
                        (float)screenW  / world.texWidth,
                        (float)screenH / world.texHeight
                    ) * world.scale;

                    if (tocaLimiteSuperior(world)) world.offsetY = 0;
                    if (tocaLimiteInferior(world, screenH)) world.offsetY = screenH - world.texHeight * world.finalScale;
                    if (tocaLimiteDerecha(world, screenW)) world.offsetX =+ world.texHeight * world.finalScale;
                    if (tocaLimiteIzquierda(world, screenW)) world.offsetX = 0;
                break;
            }
            //==================================================================================================================
            // pan
            //==================================================================================================================
            case SDL_MOUSEMOTION: {

                if (!world.dragging) {
                    break;
                }
                int screenW, screenH;
                SDL_GetWindowSize(window, &screenW, &screenH);

                world.offsetY += event.motion.y - world.lastY;
                world.offsetX += event.motion.x - world.lastX;

                if (tocaLimiteSuperior(world)) world.offsetY = 0;
                if (tocaLimiteInferior(world, screenH)) world.offsetY = screenH - world.texHeight * world.finalScale;
                if (tocaLimiteDerecha(world, screenW)) world.offsetX =+ world.texHeight * world.finalScale;
                if (tocaLimiteIzquierda(world, screenW)) world.offsetX = 0 ;

                world.lastX = event.motion.x;
                world.lastY = event.motion.y;
                
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

            case SDL_QUIT:

                world.running = false;

                break;

            
        }
    }
};