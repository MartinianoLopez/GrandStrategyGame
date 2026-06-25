#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <algorithm>
#include "../Model/World.hpp"
#include "../utils.hpp"
#include "../Simulation/Army.hpp"

struct GameWindow {
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;

    GameWindow() {
        window   = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    // =========================================================================
    // helpers
    // =========================================================================

    void getScreenSize(int& w, int& h) const {
        SDL_GetWindowSize(window, &w, &h);
    }

    float computeScale(const World& world) const {
        int w, h;
        getScreenSize(w, h);
        return std::min((float)w / world.texWidth,
                        (float)h / world.texHeight) * world.scale;
    }

    // Maps a screen coordinate to a texture coordinate, wrapping horizontally.
    // Returns false if the y coordinate is outside the texture bounds.
    bool screenToTexture(const World& world, float sx, float sy, int& texX, int& texY) const {
        float scale = computeScale(world);
        texX = static_cast<int>((sx - world.offsetX) / scale);
        texY = static_cast<int>((sy - world.offsetY) / scale);
        texX = ((texX % world.texWidth) + world.texWidth) % world.texWidth; // horizontal wrap
        return texY >= 0 && texY < world.texHeight;
    }

    Province* pickProvince(const World& world, float sx, float sy) const {
        int texX, texY;
        if (!screenToTexture(world, sx, sy, texX, texY)) return nullptr;
        uint32_t color = getPixelColor(world.provincesBmp, texX, texY);
        return provinceFindByColor(world.provinces, color);
    }

    // =========================================================================
    // bounds clamping (only applied when freecamera is off)
    // =========================================================================

    void clampToBounds(World& world) {
        int screenW, screenH;
        getScreenSize(screenW, screenH);

        float mapW = world.texWidth  * world.finalScale;
        float mapH = world.texHeight * world.finalScale;

        if (world.offsetY > 0)                  world.offsetY = 0;
        if (world.offsetY < screenH - mapH)     world.offsetY = screenH - mapH;
        if (world.offsetX < screenW - mapW)     world.offsetX = screenW;
        if (world.offsetX > mapW)               world.offsetX = 0;
    }

    // =========================================================================
    // input handlers
    // =========================================================================

    void onScroll(World& world, const SDL_Event& e) {
        float zoom = (e.wheel.y > 0) ? 1.1f : 0.9f;
        float mx   = (float)e.wheel.mouseX;
        float my   = (float)e.wheel.mouseY;

        bool tooZoomedOut = !world.freecamera && world.scale * zoom <= 1.5f;
        if (tooZoomedOut) return;

        // Zoom toward the mouse cursor
        world.offsetX = mx + (world.offsetX - mx) * zoom;
        world.offsetY = my + (world.offsetY - my) * zoom;
        world.scale  *= zoom;

        if (!world.freecamera) {
            world.finalScale = computeScale(world);
            clampToBounds(world);
        }
    }

    void onMouseMove(World& world, const SDL_Event& e) {
        if (!world.dragging) return;

        world.offsetX += e.motion.x - world.lastX;
        world.offsetY += e.motion.y - world.lastY;
        world.lastX    = e.motion.x;
        world.lastY    = e.motion.y;

        if (!world.freecamera)
            clampToBounds(world);
    }



    void onLeftClick(World& world, const SDL_Event& e) {        
        world.dragging = true;
        world.lastX    = e.button.x;
        world.lastY    = e.button.y;

        int w, h;
        getScreenSize(w, h);

        // Try click on UI Layer
        for (auto it = world.uiElements.rbegin(); it != world.uiElements.rend(); ++it) {
            if (it->contains(e.button.x, e.button.y, w, h)) {
                if (it->onClick) it->onClick();
                return; // consumed
            }
        }

        // click on map layer
        Province* target = pickProvince(world, e.button.x, e.button.y);
        if (!target) {
            int texX, texY;
            screenToTexture(world, e.button.x, e.button.y, texX, texY);
            std::cerr << "No province at color: " << colorToString(getPixelColor(world.provincesBmp, texX, texY)) << "\n"; 
            return;
        }        
        
        world.selectedProvince = target->id;
        world.selectedCountry = provinceFindById(world.provinces, world.selectedProvince) -> owner;
        // ====================== army selection ============================================

        if (world.place == MenuPlace::InGame){

            world.selectedArmies.clear();
            
            Army* army = FindArmyOnProvinceId(world.armies, world.selectedProvince);

            if (!army)                                 return;
            if (!(army->owner == world.playerCountry)) return;

            world.selectedArmies.push_back(army);

        }

        // ====================== menu country selection ====================================

        if (world.place == MenuPlace::CountrySelection){

            Province* p = provinceFindById(world.provinces, world.selectedProvince);
            Country* c = findCountryByTag(world.countries, p->owner);

            if (c) world.playerCountry = c->tag;

            std::string path = "assets/flags/" + world.playerCountry + ".tga";
            world.flagTex = IMG_LoadTexture(renderer, path.c_str());

            if (world.flagTex) world.flagTex = world.flagTex;
        }
    }    
    
    void onRightClick(World& world, const SDL_Event& e) {
        
        Province* clickedProvince = pickProvince(world, e.button.x, e.button.y);
        if (!clickedProvince) return;
        world.objectiveProvince = clickedProvince->id;

        // ====================== army recluitment =====================
        if(world.recluitOneUnit == true){

            Country* country = findCountryByTag(world.countries, world.playerCountry);
            if(country -> money >= 100){
                country -> money -= 100;
                world.armies.push_back(Army(world.objectiveProvince, "Recluits", world.playerCountry, 1000,country->color));
            }
            world.recluitOneUnit = false;
        }
        // ====================== army movement ========================
        if (world.selectedArmies.empty() || world.selectedArmies[0] == nullptr) return;
        for (Army* army : world.selectedArmies) {
            createArmyMovement(world, army, army->position, world.objectiveProvince);
        }
        // instant movement army->position = target->id;

    }

    // =========================================================================
    // event dispatch
    // =========================================================================
    void processEvent(World& world, const SDL_Event& e) {
        switch (e.type) {
            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    int w, h;
                    getScreenSize(w, h);

                    float oldScale = world.finalScale;
                    // texel bajo el centro antes del resize
                    float texCX = (w / 10.0f - world.offsetX) / oldScale;
                    float texCY = (h / 10.0f - world.offsetY) / oldScale;

                    world.finalScale = computeScale(world);

                    // reposicionar para que ese texel siga bajo el centro
                    world.offsetX = w / 10.0f - texCX * world.finalScale;
                    world.offsetY = h / 10.0f - texCY * world.finalScale;

                    if (!world.freecamera)
                        clampToBounds(world);
                }
                break;
            case SDL_MOUSEWHEEL:
                if (world.place == MenuPlace::InGame || world.place == MenuPlace::CountrySelection )
                {
                    onScroll(world, e);
                }
                break;

            case SDL_MOUSEMOTION:
                if (world.place == MenuPlace::InGame || world.place == MenuPlace::CountrySelection)
                {
                    onMouseMove(world, e);
                } 
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_RIGHT) {
                    onRightClick(world, e);
                }

                if (e.button.button == SDL_BUTTON_LEFT) {
                    onLeftClick(world, e);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    world.dragging = false;
                }
                break;

            case SDL_QUIT:
                world.running = false;
                break;
        }
    }
};