#pragma once

//=============================

#include "../utils.hpp"
#include "../Model/World.hpp"
#include "MapModeBuilders.hpp"

//=============================

#include "SDL_render.h"
#include <string>

inline void renderNormalMap(World& world) {
    displayTexture(world, world.countriesTex, 245);
    displayTexture(world, world.controlTex, 245);
}

inline void renderAccessMap(World& world) {
    displayTexture(world, world.countriesTex, 100);
    if (!world.activeAccessibilityMap || world.selectedCountry != world.countryoftheAccesibilityMap) {
        SDL_DestroyTexture(world.activeAccessibilityMap);
        world.activeAccessibilityMap = nullptr;
        
        Country* country = findCountryByTag(world.countries, world.selectedCountry);
        if (country) {
            world.activeAccessibilityMap = buildAccessibilityMap(world, country->accessibleCountries);
        }
        world.countryoftheAccesibilityMap = world.selectedCountry;
    }
    
    if (world.activeAccessibilityMap) {
        displayTexture(world, world.activeAccessibilityMap, 245);
    }
}

inline void renderDiplomaticMap(World& world) {
    displayTexture(world, world.countriesTex, 100);
    if (!world.activeDiplomaticMap || world.selectedCountry != world.countryoftheAccesibilityMap) {
        SDL_DestroyTexture(world.activeDiplomaticMap);
        world.activeDiplomaticMap = nullptr;
        
        Country* country = findCountryByTag(world.countries, world.selectedCountry);
        if (country) {
            world.activeDiplomaticMap = buildDiplomaticMap(world, world.renderer, country->relationships);
        }
        world.countryoftheAccesibilityMap = world.selectedCountry;
    }

    if (world.activeDiplomaticMap) { 
        displayTexture(world, world.activeDiplomaticMap, 245); 
    }
}