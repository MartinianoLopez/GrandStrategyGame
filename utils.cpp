#include "utils.hpp"

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