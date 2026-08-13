#pragma once

//===============================

#include "../Model/World.hpp"
#include "../utils.hpp"
#include "../Simulation/Army.hpp"

//===============================

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cstddef>
#include <iostream>
#include <algorithm>

//===============================

inline float computeScale(const World& world) {
    int w, h;
    SDL_GetWindowSize(world.window, &w, &h);
    return std::min((float)w / world.texWidth,(float)h / world.texHeight) * world.scale;
}

// Maps a screen coordinate to a texture coordinate, wrapping horizontally.
// Returns false if the y coordinate is outside the texture bounds.
inline bool screenToTexture(const World& world, float sx, float sy, int& texX, int& texY) {
    float scale = computeScale(world);
    texX = static_cast<int>((sx - world.offsetX) / scale);
    texY = static_cast<int>((sy - world.offsetY) / scale);
    texX = ((texX % world.texWidth) + world.texWidth) % world.texWidth; // horizontal wrap
    return texY >= 0 && texY < world.texHeight;
}

inline Province* pickProvince(const World& world, float sx, float sy) {
    int texX, texY;
    if (!screenToTexture(world, sx, sy, texX, texY)) return nullptr;
    uint32_t color = getPixelColor(world.provincesBmp, texX, texY);
    return provinceFindByColor(world.provinces, color);
}

// =========================================================================
// bounds clamping (only applied when freecamera is off)
// =========================================================================

inline void clampToBounds(World& world) {
    int screenW, screenH;
    SDL_GetWindowSize(world.window, &screenW, &screenH);

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

inline void onScroll(World& world, const SDL_Event& e) {
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

inline void onMouseMove(World& world, const SDL_Event& e) {
    if (!world.dragging) return;

    world.offsetX += e.motion.x - world.lastX;
    world.offsetY += e.motion.y - world.lastY;
    world.lastX    = e.motion.x;
    world.lastY    = e.motion.y;

    if (!world.freecamera){
        clampToBounds(world);
    }
}

inline void onLeftClick(World& world, const SDL_Event& e) {        
    world.dragging = true;
    world.lastX    = e.button.x;
    world.lastY    = e.button.y;

    int screenW, screenH;
    SDL_GetWindowSize(world.window, &screenW, &screenH);

    // Try click on UI Layer
    for (auto it = world.ui.uiElements.rbegin(); it != world.ui.uiElements.rend(); ++it) {
        if (it->contains(e.button.x, e.button.y, screenW, screenH)) {
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
        
    if( world.selectedCountry != "NONE" ){
        world.ui.Textures["selectedCountryFlagTex"] = findCountryByTag(world.countries, world.selectedCountry) -> flag;
    }else{
        world.ui.Textures["selectedCountryFlagTex"] = NULL;
    }
        
// ====================== army selection ============================================

    if (world.ui.place == MenuPlace::InGame){

        world.selectedArmies.clear();
            
        Army* army = FindArmyOnProvinceId(world.armies, world.selectedProvince);

        if (!army)                                 return;
        if (!(army->owner == world.playerCountry)) return;

        world.selectedArmies.push_back(army);

    }

// ====================== country selection ====================================

    if (world.ui.place == MenuPlace::CountrySelection){

        Province* province = provinceFindById(world.provinces, world.selectedProvince);
        Country* country = findCountryByTag(world.countries, province->owner);

        if (country){
            world.playerCountry = country->tag;
            world.ui.Textures["PlayerflagTexture"] = world.ui.Textures["selectedCountryFlagTex"];
            world.ui.Textures["selectedCountryFlagTex"] = country->flag;
        }
    }
}    
    
inline void onRightClick(World& world, const SDL_Event& e) {
        
    Province* clickedProvince = pickProvince(world, e.button.x, e.button.y);
    if (!clickedProvince) return;
    world.objectiveProvince = clickedProvince->id;

    // ====================== army recruitment =====================
    if(world.recruitOneUnit == true){
        tryToRecruitArmy(world);
    }
    // ====================== army movement ========================
    if (world.selectedArmies.empty() || world.selectedArmies[0] == nullptr) return;
    for (Army* army : world.selectedArmies) {
        createArmyMovement(world, army, army->position, world.objectiveProvince);
    }
}
    
// =========================================================================
// event dispatch
// =========================================================================

inline void processEvent(World& world, const SDL_Event& e) {
    switch (e.type) {
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                int screenW, screenH;
                SDL_GetWindowSize(world.window, &screenW, &screenH);

                float oldScale = world.finalScale;
                // texture over the center before resize
                float texCX = (screenW / 10.0f - world.offsetX) / oldScale;
                float texCY = (screenH / 10.0f - world.offsetY) / oldScale;

                world.finalScale = computeScale(world);

                // reposition the texture to keep it on the center
                world.offsetX = screenW / 10.0f - texCX * world.finalScale;
                world.offsetY = screenH / 10.0f - texCY * world.finalScale;

                if (!world.freecamera){
                    clampToBounds(world);
                }
            }
        break;

        case SDL_MOUSEWHEEL:
            if (world.ui.place == MenuPlace::InGame || world.ui.place == MenuPlace::CountrySelection )
            {
                onScroll(world, e);
            }
        break;

        case SDL_MOUSEMOTION:
            if (world.ui.place == MenuPlace::InGame || world.ui.place == MenuPlace::CountrySelection)
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