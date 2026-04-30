#pragma once
#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <SDL2/SDL_ttf.h>

struct GameData {
    GameData();
    SDL_Surface* provincesBmp;
    SDL_Texture* terrain;
    SDL_Texture* height;
    SDL_Surface* countries;
    std::map<uint32_t, uint32_t> BmpColorToProvinceId;
    std::map<uint32_t, std::string> ProvinceIdToCountryTag;
    std::map<std::string, std::string> CountryTagToCountryName;
    std::map<std::string, uint32_t> CountryNameToCountryColor;
    

    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_Point>> frontierList;
    SDL_Texture* frontierTexture;

    std::map<uint32_t, SDL_Point> ProvincesCenterList;       // provinceColor, centerOfTheProvince
    std::map<uint32_t, uint32_t> troopsList;                 // provinceId, Cuantity

    int texWidth;
    int texHeight;

    float scale;
    float offsetX;
    float offsetY;

    bool dragging;
    int lastX, lastY;

    int frameDelay;

    uint32_t selectedProvince;
    uint32_t selectedProvinceId;
    uint32_t secundarySelectedProvinceId;
    int id;
    float fps = 0.0f;
    TTF_Font* font = nullptr;
    SDL_Texture* digits[10];
    float finalScale;
};