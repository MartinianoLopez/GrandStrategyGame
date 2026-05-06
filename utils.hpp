#pragma once
#include <map>
#include <optional>
#include <string>
#include <iostream>
#include <SDL2/SDL.h>
#include "GameData.hpp"

template <typename Map>
auto mapFind(const Map& map, const typename Map::key_type& key)
    -> std::optional<typename Map::mapped_type>
{
    auto it = map.find(key);
    if (it == map.end()) return std::nullopt;
    return it->second;
}
std::optional<uint32_t> mapFind(const std::map<std::string, uint32_t>& map, const std::string& key) {
    auto it = map.find(key);
    if (it == map.end()) return std::nullopt;
    return it->second;
}

template <typename Map>
void showMap(const Map& map) {
    int count = 0;
    for (const auto& [key, value] : map) {
        if (count++ >= 10) break;
        std::cout << key << " -> " << value << "\n";
    }
}

// ===============================================================================================================
// uint32_t ---> "r, g, b"
// ===============================================================================================================
std::string colorToString(uint32_t color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8)  & 0xFF;
    uint8_t b = (color)       & 0xFF;
    return "" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + "";
}

// ===============================================================================================================
// ProvinceColor → ProvinceId → CountryTag → CountryName → CountryColor
// ===============================================================================================================
uint32_t getCountryColorFromProvinceColor(const GameData& state, uint32_t provinceColor) {
    auto provinceId = mapFind(state.BmpColorToProvinceId, provinceColor);
    if (!provinceId)   return 0;
    auto countryTag = mapFind(state.ProvinceIdToCountryTag, *provinceId);
    if (!countryTag)   return 0;
    auto countryName = mapFind(state.CountryTagToCountryName, *countryTag);
    if (!countryName)  return 0;
    auto countryColor = mapFind(state.CountryNameToCountryColor, *countryName);
    return countryColor.value_or(0);
}

// ===============================================================================================================
// SDL_Surface ---> uint32_t pixel color at (x, y)
// ===============================================================================================================
uint32_t getPixelColor(SDL_Surface* surface, int x, int y) {
    uint8_t* pixel = (uint8_t*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
    return *(uint32_t*)pixel;
}
SDL_Texture* surfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface) {
    if (!surface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    return texture;
}
uint32_t searchTroops(const GameData& state, uint32_t provinceId) {
    auto it = state.troopsList.find(provinceId);
    if (it != state.troopsList.end()) return it->second;
    return 0; // sin tropas
}
void moveTroops(GameData& state, uint32_t fromProvince, uint32_t toProvince) {
    if (fromProvince == toProvince) return;
    if (!state.troopsList.count(fromProvince)) return;
    state.troopsList[toProvince] += state.troopsList[fromProvince];
    state.troopsList.erase(fromProvince);
    state.selectedProvince = 0;
}