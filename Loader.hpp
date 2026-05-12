#pragma once
#include <string>
#include "World.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <iostream>
#include <SDL2/SDL_image.h>
#include "utils.hpp"
#include <chrono>

// ===============================================================================================================
// frontiers
// ===============================================================================================================
std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> findFrontiers(SDL_Surface* img) {   
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> frontierList;
    int imgW = img->w;
    int imgH = img->h;

    for (int y = 0; y < imgH; y++)
    for (int x = 0; x < imgW; x++) {
        uint32_t current = getPixelColor(img, x, y);
        if (x + 1 < imgW) { 
            uint32_t next = getPixelColor(img, x + 1, y);
            if (next != current) frontierList[{current, next}].push_back({x + 0.5f, (float)y});
        }
    }

    for (int y = 0; y < imgH; y++) 
    for (int x = 0; x < imgW; x++) {  
        uint32_t current = getPixelColor(img, x, y);
        if (y + 1 < imgH) {  
            uint32_t next = getPixelColor(img, x, y + 1);
            if (next != current) frontierList[{current, next}].push_back({(float)x, y + 0.5f});
        }
    }

    return frontierList;
}

std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> findFrontiersBetweenCountries(
    World& world,
    const std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>>& frontiers)
{
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> filteredFrontiers;

    for (const auto& [key, points] : frontiers) {
        const Province* province1 = provinceFindByColor(world.provinces, key.first);
        const Province* province2 = provinceFindByColor(world.provinces, key.second);

        if (province1 == nullptr || province2 == nullptr) continue;

        const bool differentCountries = province1->owner != province2->owner
                                     && !province1->owner.empty()
                                     && !province2->owner.empty();

        const bool oneIsCountryOneIsNot = province1->owner.empty() != province2->owner.empty();

        if (differentCountries || oneIsCountryOneIsNot) {
            filteredFrontiers[key] = points;
        }
    }

    return filteredFrontiers;
}



std::map<uint32_t, SDL_Point> initProvincesCenters(const World& world) {

    struct Accum {
        int sumX = 0;
        int sumY = 0;
        int count = 0;
    };

    std::map<uint32_t, Accum> accum;

    for (int y = 0; y < world.texHeight; y++) {

        for (int x = 0; x < world.texWidth; x++) {

            uint32_t color =
                getPixelColor(world.provincesBmp, x, y);

            auto& a = accum[color];

            a.sumX += x;
            a.sumY += y;
            a.count++;
        }
    }

    std::map<uint32_t, SDL_Point> centerList;

    for (auto& [color, a] : accum) {

        if (a.count > 0) {

            centerList[color] = {
                a.sumX / a.count,
                a.sumY / a.count
            };
        }
    }

    return centerList;
}

// ===============================================================================================================
// helpers
// ===============================================================================================================
std::string extractValue(const std::string& line, const std::string& key) {
    // busca "key": "value" o "key": number
    size_t pos = line.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = line.find(":", pos);
    if (pos == std::string::npos) return "";
    pos++;
    while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) pos++;
    if (line[pos] == '"') {
        pos++;
        size_t end = line.find('"', pos);
        return line.substr(pos, end - pos);
    } else {
        size_t end = pos;
        while (end < line.size() && line[end] != ',' && line[end] != '}') end++;
        std::string val = line.substr(pos, end - pos);
        val.erase(val.find_last_not_of(" \t\r\n") + 1);
        return val;
    }
}

// ===============================================================================================================
// loaders
// ===============================================================================================================

std::list<Province> loadProvinces(
    const std::map<uint32_t, SDL_Point>& centerList
) {

    std::list<Province> provinces;

    std::ifstream file("assets/provinces.txt");

    if (!file.is_open()) {
        std::cerr << "Error: could not open provinces.txt\n";
        return provinces;
    }

    // format:
    // id;r;g;b;name;owner

    std::string line;

    // skip header
    std::getline(file, line);

    while (std::getline(file, line)) {

        std::stringstream ss(line);

        std::string token;

        std::vector<std::string> parts;

        while (std::getline(ss, token, ';')) {
            parts.push_back(token);
        }

        if (parts.size() < 6)
            continue;

        int id = std::stoi(parts[0]);

        int r = std::stoi(parts[1]);
        int g = std::stoi(parts[2]);
        int b = std::stoi(parts[3]);

        std::string name = parts[4];
        std::string owner = parts[5];

        Province p(
            id,
            name,
            owner,
            Color(r, g, b)
        );

        uint32_t colorKey =
            ((uint32_t)r << 16) |
            ((uint32_t)g << 8) |
            (uint32_t)b;

        auto it = centerList.find(colorKey);

        if (it != centerList.end()) {
            p.center = it->second;
        }

        provinces.push_back(std::move(p));
    }

    return provinces;
}

// ===============================================================================================================

std::list<Country> loadCountries() {

    std::list<Country> countries;

    std::ifstream file("assets/countries.txt");

    if (!file.is_open()) {
        std::cerr << "Error: could not open countries.txt\n";
        return countries;
    }

    // format:
    // tag;name;r;g;b;money

    std::string line;

    // skip header
    std::getline(file, line);

    while (std::getline(file, line)) {

        std::stringstream ss(line);

        std::string token;

        std::vector<std::string> parts;

        while (std::getline(ss, token, ';')) {
            parts.push_back(token);
        }

        if (parts.size() < 6)
            continue;

        std::string tag = parts[0];
        std::string name = parts[1];

        int r = std::stoi(parts[2]);
        int g = std::stoi(parts[3]);
        int b = std::stoi(parts[4]);

        int money = std::stoi(parts[5]);

        countries.emplace_back(
            tag,
            name,
            Color(r, g, b),
            money
        );
    }

    return countries;
}

// ===============================================================================================================

std::list<Army> loadArmies() {

    std::list<Army> armies;

    std::ifstream file("assets/armies.txt");

    if (!file.is_open()) {
        std::cerr << "Error: could not open armies.txt\n";
        return armies;
    }

    // format:
    // id;name;owner;power

    std::string line;

    // skip header
    std::getline(file, line);

    while (std::getline(file, line)) {

        std::stringstream ss(line);

        std::string token;

        std::vector<std::string> parts;

        while (std::getline(ss, token, ';')) {
            parts.push_back(token);
        }

        if (parts.size() < 4)
            continue;

        int provinceId = std::stoi(parts[0]);

        std::string name = parts[1];

        std::string owner = parts[2];

        int power = std::stoi(parts[3]);

        armies.emplace_back(
            provinceId,
            name,
            owner,
            power
        );
    }

    return armies;
}

SDL_Surface* prepareCountries(const World& world) {
    auto start = std::chrono::high_resolution_clock::now();

    SDL_Surface* provinces = world.provincesBmp;
    if (!provinces) return nullptr;

    SDL_Surface* result = SDL_CreateRGBSurfaceWithFormat(
        0, provinces->w, provinces->h, 32, provinces->format->format
    );
    if (!result) return nullptr;

    std::map<uint32_t, uint32_t> colorToCountryColor;

    SDL_LockSurface(provinces);
    SDL_LockSurface(result);

    int imgW = provinces->w;
    int imgH = provinces->h;

    for (int y = 0; y < imgH; y++) {
        for (int x = 0; x < imgW; x++) {
            uint32_t pixelColor = getPixelColor(provinces, x, y);

            auto it = colorToCountryColor.find(pixelColor);
            if (it != colorToCountryColor.end()) {
                if (it->second != 0) setPixel(result, x, y, it->second);
                continue;
            }

            Province* p = provinceFindByColor(world.provinces, pixelColor);
            if (!p || p->owner.empty()) {
                colorToCountryColor[pixelColor] = 0;
                continue;
            }

            Country* c = countryTagFind(world.countries, p->owner);
            if (!c) {
                colorToCountryColor[pixelColor] = 0;
                continue;
            }

            uint32_t countryColor = ((uint32_t)c->color.r << 16) |
                                    ((uint32_t)c->color.g << 8)  |
                                     (uint32_t)c->color.b;

            colorToCountryColor[pixelColor] = countryColor;
            setPixel(result, x, y, countryColor);
        }
    }

    SDL_UnlockSurface(provinces);
    SDL_UnlockSurface(result);

    auto end = std::chrono::high_resolution_clock::now();
    float ms = std::chrono::duration<float, std::milli>(end - start).count();
    std::cerr << "prepareCountries: " << ms << " ms\n";

    return result;
}
void initFont(World& world, SDL_Renderer* renderer, const char *fontPath) {
    if (TTF_Init() < 0) return;

    // Cargar a doble resolución
    world.font = TTF_OpenFont(fontPath, 64);
    if (!world.font) return;

    for (int i = 0; i < 10; i++) {
        SDL_Surface* s = TTF_RenderText_Blended(
            world.font,
            std::to_string(i).c_str(),
            {255, 255, 255, 255}
        );
        if (!s) continue;
        world.digits[i] = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
    }
}
// ===============================================================================================================
// LOAD ALL
// ===============================================================================================================
void loadAssets(World& world, SDL_Renderer* renderer) {
    auto start = std::chrono::high_resolution_clock::now();
    
    world.provincesBmp = IMG_Load("assets/terrain/provinces.bmp");
    if (!world.provincesBmp) {
        std::cerr << "Error loading provinces.bmp: " << IMG_GetError() << "\n";
        return;
    }

    world.terrain = surfaceToTexture(renderer, IMG_Load("assets/terrain/terrain.bmp"));
    world.height  = surfaceToTexture(renderer, IMG_Load("assets/terrain/heightmap.bmp"));

    world.texWidth  = world.provincesBmp->w;
    world.texHeight = world.provincesBmp->h;

    std::map<uint32_t, SDL_Point> centerList = initProvincesCenters(world);

    world.provinces = loadProvinces(centerList);
    world.countries = loadCountries();
    world.armies    = loadArmies();

    world.countriesImg = prepareCountries(world);

    world.provinceFrontiers = findFrontiers(world.provincesBmp);
    world.countryFrontiers  = findFrontiersBetweenCountries(world, world.provinceFrontiers);
    
    initFont(world, renderer,"assets/Cinzel_Decorative/CinzelDecorative-Regular.ttf");
    world.playerCountry = "GBR";

    auto end = std::chrono::high_resolution_clock::now();
    float ms = std::chrono::duration<float, std::milli>(end - start).count();
    std::cerr << "time: " << ms << " ms\n";
}