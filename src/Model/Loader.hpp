#pragma once

// ========================

#include "World.hpp"
#include "DataProcessing.hpp"
#include "DataLoading.hpp"
#include "../utils.hpp"
#include "../utils/Timer.hpp"

// ========================

#include <cstdint>
#include <string>
#include "SDL_pixels.h"
#include "SDL_render.h"
#include <algorithm>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_image.h>

// ========================

inline std::unordered_map<int, std::vector<std::pair<uint16_t, uint16_t>>> buildShapes(World& world) {
    SDL_Surface* surface = world.provincesBmp;

    std::unordered_map<uint32_t, int> colorToId;
    for (const auto& pd : world.provincesData) {
        uint32_t key = ((uint32_t)pd.color.r << 16) | ((uint32_t)pd.color.g << 8) | pd.color.b;
        colorToId[key] = pd.id;
    }

    std::unordered_map<int, std::vector<std::pair<uint16_t, uint16_t>>> shapeMap;

    SDL_LockSurface(surface);
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            SDL_Color c = getPixel(surface, x, y);
            uint32_t key = ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | c.b;

            auto it = colorToId.find(key);
            if (it != colorToId.end())
                shapeMap[it->second].emplace_back(x, y);
        }
    }
    SDL_UnlockSurface(surface);
    return shapeMap;
}

inline void loadProvinces(World& world) {
    loadProvincesTxt(world);
    auto shapes = buildShapes(world);
    // check that they exit here
    auto centers = initProvincesCenters(world);

    std::list<Province> provinces;
    for (const auto& pd : world.provincesData) {
        Color color(pd.color.r, pd.color.g, pd.color.b);
        Province p(pd.id, pd.name, pd.owner, color);

        uint32_t key = ((uint32_t)pd.color.r << 16) | ((uint32_t)pd.color.g << 8) | pd.color.b;

        auto it = shapes.find(pd.id);
        if (it != shapes.end())
            p.shape = it->second;

        auto ic = centers.find(key);
        if (ic != centers.end())
            p.center = ic->second;

        provinces.push_back(p);
    }
    world.provinces = provinces;
}

inline void desaturateCountries(std::list<Country>& countries, double k = 0.3, int brightness = 20) {
    for (auto& c : countries) {
        int r = c.color.r;
        int g = c.color.g;
        int b = c.color.b;

        double gray = (r + g + b) / 3.0;

        int newR = static_cast<int>(r + (gray - r) * k) + brightness;
        int newG = static_cast<int>(g + (gray - g) * k) + brightness;
        int newB = static_cast<int>(b + (gray - b) * k) + brightness;

        newR = std::clamp(newR, 0, 255);
        newG = std::clamp(newG, 0, 255);
        newB = std::clamp(newB, 0, 255);

        c.color = Color(newR, newG, newB);
    }
}

// ===============================================================================================================
// font
// ===============================================================================================================

inline Font initFont(SDL_Renderer* renderer, const std::string& id, const char* fontPath, SDL_Color color, int fontSize) {
    Font font;
    font.id = id;

    TTF_Font* ttf = TTF_OpenFont(fontPath, fontSize);
    if (!ttf) {
        SDL_Log("Font load error: %s", TTF_GetError());
        return font;
    }

    for (int c = 32; c < 127; c++) {
        char text[2] = { (char)c, '\0' };
        SDL_Surface* s = TTF_RenderText_Solid(ttf, text, color);
        if (!s) continue;

        font.glyphs[c].tex = SDL_CreateTextureFromSurface(renderer, s);
        font.glyphs[c].w = s->w;
        font.glyphs[c].h = s->h;

        SDL_FreeSurface(s);
    }

    TTF_CloseFont(ttf);
    return font;
}


// ===============================================================================================================
// optimization
// ===============================================================================================================

inline void buildProvinceIdMap(World& world) {
    world.provinceById.clear();
    for (auto& province : world.provinces)
        world.provinceById[province.id] = &province;
}

// ===============================================================================================================
// LOAD ALL
// ===============================================================================================================

static std::string FOLDERPATH = "assets/terrainCustom/";
static float STARTING_COORDINATES[] = {0.57f, 0.22f};

inline void loadAssets(World& world) {
    SDL_Renderer* renderer = world.renderer;

    world.provincesBmp = IMG_Load((FOLDERPATH + "provinces.bmp").c_str());
    world.terrain = surfaceToTexture(renderer, IMG_Load((FOLDERPATH + "terrain.bmp").c_str()));
    world.height  = surfaceToTexture(renderer, IMG_Load((FOLDERPATH + "heightmap.bmp").c_str()));

    world.texWidth  = world.provincesBmp->w;
    world.texHeight = world.provincesBmp->h;
    
    // load files

    { Timer t("Provinces"); loadProvinces(world); }
    { Timer t("Countries"); loadCountries(world); }
    { Timer t("Armies");    loadArmies(world); }
    
    // data processing

    { Timer t("ProcessColors"); desaturateCountries(world.countries, 0.3, 5); }
    { Timer t("PrepareCountries"); prepareCountries(world); }

    world.countriesTex = surfaceToTexture(renderer, world.countriesImg);

    world.controlSur = SDL_CreateRGBSurfaceWithFormat(0, world.provincesBmp->w, world.provincesBmp->h, 32, SDL_PIXELFORMAT_RGBA32);

    world.controlTex = surfaceToTexture(renderer, world.controlSur);

    { Timer t("FindFrontiers");         findFrontiers(world); }
    { Timer t("FindCountryFrontiers");  findFrontiersBetweenCountries(world); }
    { Timer t("BuildAdjacency");        buildAdjacency(world); }
    { Timer t("BuildProvinces"); buildProvinceIdMap(world); }
    { Timer t("BuildAccesibilityGraphs"); InitAllAccesibiltyGraphs(world); }

    if (TTF_Init() == -1) {
        SDL_Log("TTF init error: %s", TTF_GetError());
        return;
    }

    world.fonts.push_back(initFont(renderer, "army", "assets/fonts/Nunito/Nunito-VariableFont_wght.ttf", {0, 0, 0, 255}, 12));
    world.fonts.push_back(initFont(renderer, "simple", "assets/fonts/Cinzel/static/Cinzel-Medium.ttf", {0, 0, 0, 255}, 18));
    world.fonts.push_back(initFont(renderer, "fancy", "assets/fonts/Cinzel/Cinzel-VariableFont_wght.ttf", {220, 220, 220, 255}, 22));

    world.finalScale = std::min(1920.0f / world.texWidth,1080.0f / world.texHeight) * world.scale;

    // center starts over europe 
    world.offsetX = STARTING_COORDINATES[0] * (1920 - world.texWidth * world.finalScale);
    world.offsetY = STARTING_COORDINATES[1] * (1080 - world.texHeight * world.finalScale);
}
