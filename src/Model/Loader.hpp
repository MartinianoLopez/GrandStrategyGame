#pragma once

// ========================

#include "World.hpp"
#include "DataProcessing.hpp"
#include "DataLoading.hpp"
#include "../utils.hpp"
#include "../utils/Timer.hpp"
#include "FontLoader.hpp"

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

    // Maps each color (packed as 0xRRGGBB) to its corresponding province id,
    // so we can look up a pixel's province in O(1).
    std::unordered_map<uint32_t, int> colorToId;
    colorToId.reserve(world.provincesData.size() * 2);
    for (const auto& pd : world.provincesData) {
        uint32_t key = ((uint32_t)pd.color.r << 16) | ((uint32_t)pd.color.g << 8) | pd.color.b;
        colorToId[key] = pd.id;
    }

    // Temporary buckets indexed by province id, holding all pixel coordinates
    // that belong to that province.
    int maxId = 0;
    for (auto& pd : world.provincesData) maxId = std::max(maxId, pd.id);
    std::vector<std::vector<std::pair<uint16_t, uint16_t>>> buckets(maxId + 1);

    SDL_LockSurface(surface); // gives direct access to surface->pixels

    const int w = surface->w, h = surface->h;
    const int bpp = surface->format->BytesPerPixel; // bytes per pixel (1, 2, 3 or 4)
    const int pitch = surface->pitch;                // bytes per row (may include padding)
    const uint8_t* pixels = static_cast<const uint8_t*>(surface->pixels);
    const SDL_PixelFormat* fmt = surface->format;

    for (int y = 0; y < h; y++) {
        const uint8_t* row = pixels + y * pitch; // pointer to the start of row y

        for (int x = 0; x < w; x++) {
            uint32_t pixel;
            const uint8_t* p = row + x * bpp; // pointer to pixel (x, y)

            // Reads the raw pixel value according to its byte size.
            switch (bpp) {
                case 1: pixel = *p; break;
                case 2: pixel = *reinterpret_cast<const uint16_t*>(p); break;
                case 3:
                    // 3-byte pixels aren't naturally aligned to an integer type,
                    // so the value is assembled manually respecting endianness.
                    pixel = (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                        ? (p[0] << 16 | p[1] << 8 | p[2])
                        : (p[2] << 16 | p[1] << 8 | p[0]);
                    break;
                default: pixel = *reinterpret_cast<const uint32_t*>(p); break; // 4-byte pixel
            }

            // Converts the raw pixel value (format-dependent) into standard
            // 8-bit RGB components.
            uint8_t r, g, b;
            SDL_GetRGB(pixel, fmt, &r, &g, &b);
            uint32_t key = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;

            // If the color matches a known province, store this pixel's
            // coordinate in that province's bucket.
            auto it = colorToId.find(key);
            if (it != colorToId.end())
                buckets[it->second].emplace_back((uint16_t)x, (uint16_t)y);
        }
    }
    SDL_UnlockSurface(surface);

    // Copies the non-empty buckets into the final map that gets returned.
    // move() avoids copying the coordinate vectors.
    std::unordered_map<int, std::vector<std::pair<uint16_t, uint16_t>>> shapeMap;
    shapeMap.reserve(world.provincesData.size());
    for (int id = 0; id <= maxId; id++)
        if (!buckets[id].empty())
            shapeMap.emplace(id, std::move(buckets[id]));

    return shapeMap;
}

inline void loadProvinces(World& world) {
    { Timer t("     loadProvincesTxt"); loadProvincesTxt(world); }

    std::unordered_map<int, std::vector<std::pair<uint16_t, uint16_t>>> shapes;
    { Timer t("     buildShapes"); shapes = buildShapes(world); }

    std::map<uint32_t, SDL_Point> centers;
    { Timer t("     initcenters"); centers = initProvincesCenters(world); }

    std::list<Province> provinces;
    for (const auto& pd : world.provincesData) {
        SDL_Color color = { pd.color.r, pd.color.g, pd.color.b, 255 };
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

       c.color = SDL_Color{ (Uint8)newR, (Uint8)newG, (Uint8)newB, 255 };
    }
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

    { Timer t("   Provinces");              loadProvinces(world); }
    { Timer t("   Countries");              loadCountries(world); }
    { Timer t("   Armies");                 loadArmies(world); }
    
    // data processing
    
    { Timer t("   ProcessColors");          desaturateCountries(world.countries, 0.3, 5); }
    { Timer t("   PrepareCountries");       prepareCountries(world); }

    world.countriesTex = surfaceToTexture(renderer, world.countriesImg);

    world.controlSur = SDL_CreateRGBSurfaceWithFormat(0, world.provincesBmp->w, world.provincesBmp->h, 32, SDL_PIXELFORMAT_RGBA32);

    world.controlTex = surfaceToTexture(renderer, world.controlSur);

    { Timer t("   FindFrontiers");           findFrontiers(world); }
    { Timer t("   FindCountryFrontiers");    findFrontiersBetweenCountries(world); }
    { Timer t("   BuildAdjacency");          buildAdjacency(world); }
    { Timer t("   BuildProvinces");          buildProvinceIdMap(world); }
    { Timer t("   BuildAccesibilityGraphs"); InitAllAccesibiltyGraphs(world); }

    if (TTF_Init() == -1) {
        SDL_Log("TTF init error: %s", TTF_GetError());
        return;
    }

    initFonts(world);

    world.finalScale = std::min(1920.0f / world.texWidth,1080.0f / world.texHeight) * world.scale;

    // center starts over europe 
    world.offsetX = STARTING_COORDINATES[0] * (1920 - world.texWidth * world.finalScale);
    world.offsetY = STARTING_COORDINATES[1] * (1080 - world.texHeight * world.finalScale);
}
