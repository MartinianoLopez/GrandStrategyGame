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

template <typename Map>
void showMap(const Map& map) {
    int count = 0;
    for (const auto& [key, value] : map) {
        if (count++ >= 10) break;
        std::cout << key << " -> " << value << "\n";
    }
}

std::string colorToString (uint32_t color);
uint32_t getCountryColorFromProvinceColor (const GameData& state, uint32_t provinceColor);
uint32_t getPixelColor (SDL_Surface* surface, int x, int y);
SDL_Texture* surfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface);
uint32_t searchTroops(const GameData& state, uint32_t provinceId);
void moveTroops(GameData& state, uint32_t fromProvince, uint32_t toProvince);