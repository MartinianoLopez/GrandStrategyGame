#pragma once
#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <SDL2/SDL_ttf.h>

struct GameData {
    SDL_Surface* provincesBmp = nullptr;
    SDL_Texture* terrain = nullptr;
    SDL_Texture* height = nullptr;
    SDL_Surface* countries = nullptr;
    std::map<uint32_t, uint32_t> BmpColorToProvinceId;
    std::map<uint32_t, std::string> ProvinceIdToCountryTag;
    std::map<std::string, std::string> CountryTagToCountryName;
    std::map<std::string, uint32_t> CountryNameToCountryColor;
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> frontierList;
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> countryFrontierList;
    SDL_Texture* frontierTexture = nullptr;
    std::map<uint32_t, SDL_Point> ProvincesCenterList;
    std::map<uint32_t, uint32_t> troopsList;
    std::map<std::string, uint32_t> countryMoneyList;
    
    int texWidth = 0;
    int texHeight = 0;
    float scale = 1.f;
    float offsetX = 0.f;
    float offsetY = 0.f;
    bool dragging = false;
    int lastX = 0, lastY = 0;
    int frameDelay = 1000 / 60;
    uint32_t selectedProvince = 0;
    uint32_t selectedProvinceId = 0;
    uint32_t secundarySelectedProvinceId = 0;
    int id = 0;
    float fps = 0.0f;
    TTF_Font* font = nullptr;
    SDL_Texture* digits[10] = {};
    float finalScale = 0.f;
    std::string playerCountry = "GBR";
};