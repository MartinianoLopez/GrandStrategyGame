#pragma once
#include <string>
#include "World.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <SDL2/SDL_image.h>
#include "../utils.hpp"
#include <chrono>
#include <unordered_set>

// ===============================================================================================================
// frontiers
// ===============================================================================================================
inline std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> findFrontiers(SDL_Surface* img) {   
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> frontierList;
    int imgW = img->w;
    int imgH = img->h;

    for (int y = 0; y < imgH; y++){
        for (int x = 0; x < imgW; x++) {
            uint32_t current = getPixelColor(img, x, y);
            if (x + 1 < imgW) { 
                uint32_t next = getPixelColor(img, x + 1, y);
                if (next != current) frontierList[{current, next}].push_back({x + 0.5f, (float)y});
            }
        }
    }

    for (int y = 0; y < imgH; y++){ 
        for (int x = 0; x < imgW; x++) {  
            uint32_t current = getPixelColor(img, x, y);
            if (y + 1 < imgH) {  
                uint32_t next = getPixelColor(img, x, y + 1);
                if (next != current) frontierList[{current, next}].push_back({(float)x, y + 0.5f});
            }
        }
    }

    return frontierList;
}


inline std::map<int, std::vector<int>> buildAdjacency(
    const std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>>& provinceFrontiers,
    const std::list<Province>& provinces)
{
    std::map<int, std::vector<int>> adjacency;
    for (const auto& [pair, _] : provinceFrontiers) {
        Province* a = provinceFindByColor(provinces, pair.first);
        Province* b = provinceFindByColor(provinces, pair.second);
        if (!a || !b) continue;
        adjacency[a->id].push_back(b->id);
        adjacency[b->id].push_back(a->id);
    }
    return adjacency;
}

inline std::map<int, std::vector<int>> buildAdjacencyPerCountry(
    const World& world,
    const std::vector<std::string>& accessibleCountryTags)
{
    const std::unordered_set<std::string> accessible(
        accessibleCountryTags.begin(), accessibleCountryTags.end());

    std::unordered_map<int, Province*> idMap;
    for (auto& province : world.provinces)
        idMap[province.id] = const_cast<Province*>(&province);

    std::map<int, std::vector<int>> adjacency;

    for (const auto& [provinceId, neighbors] : world.adjacencyGraph) {
        auto itA = idMap.find(provinceId);
        if (itA == idMap.end() || !accessible.count(itA->second->owner)) continue;

        //std::cout << "Processing province: " << provinceId << " owner: " << itA->second->owner << std::endl;

        for (int neighborId : neighbors) {
            auto itB = idMap.find(neighborId);
            if (itB == idMap.end() || !accessible.count(itB->second->owner)) continue;

            adjacency[provinceId].push_back(neighborId);
        }
    }

    return adjacency;
}

inline void buildAccessibilityGraphsPerCountry(World& world)
{
    for (auto& country : world.countries) {
        country.accessibilityGraph = buildAdjacencyPerCountry(world, country.accessibleCountries);
    }
}

inline std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> findFrontiersBetweenCountries(
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



inline std::map<uint32_t, SDL_Point> initProvincesCenters(const World& world) {

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
inline std::string extractValue(const std::string& line, const std::string& key) {
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
inline std::list<Province> loadProvinces(World& world, const std::string& path) {

    auto centerList = initProvincesCenters(world);
    std::list<Province> provinces;
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Error: could not open provinces file\n";
        return provinces;
    }

    auto toInt = [](const std::string& s, int fallback = 0) -> int {
        try { return s.empty() ? fallback : std::stoi(s); }
        catch (...) { return fallback; }
    };

    std::string line;
    while (std::getline(file, line)) {

        std::istringstream ss(line);
        std::vector<std::string> parts;
        std::string token;

        while (std::getline(ss, token, ';'))
            parts.push_back(token);

        if (parts.size() < 6) continue;

        int id = toInt(parts[0]);
        int r  = toInt(parts[1]);
        int g  = toInt(parts[2]);
        int b  = toInt(parts[3]);

        Province p(id, parts[4].empty() ? "Unknown" : parts[4],
                       parts[5].empty() ? "UNK"     : parts[5],
                       Color(r, g, b));

        uint32_t key = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
        auto it = centerList.find(key);
        if (it != centerList.end())
            p.center = it->second;

        provinces.push_back(p);
    }

    return provinces;
}



// ===============================================================================================================

inline std::list<Country> loadCountries() {

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

inline std::list<Army> loadArmies(const std::string& path, World& world) {

    std::list<Army> armies;
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Error: could not open: " << path << "\n";
        return armies;
    }

    std::string line;
    while (std::getline(file, line)) {

        std::istringstream ss(line);
        std::vector<std::string> parts;
        std::string token;

        while (std::getline(ss, token, ';'))
            parts.push_back(token);

        if (parts.size() < 4) continue;

        auto toInt = [](const std::string& s, int fallback = 0) -> int {
            try { return s.empty() ? fallback : std::stoi(s); }
            catch (...) { return fallback; }
        };

        Color color = {0, 0, 0};
        Country* c = countryTagFind(world.countries, parts[2]);
        if (c) color = c->color;

        armies.emplace_back(
            toInt(parts[0]),
            parts[1],
            parts[2],
            toInt(parts[3]),
            color
        );
    }
    return armies;
}

// ===============================================================================================================

inline SDL_Surface* prepareCountries(SDL_Renderer* renderer,const World& world) {
    auto start = std::chrono::high_resolution_clock::now();

    SDL_Surface* provinces = world.provincesBmp;
    if (!provinces) return nullptr;

    SDL_Surface* result = SDL_CreateRGBSurfaceWithFormat(
        0, provinces->w, provinces->h, 32, SDL_PIXELFORMAT_RGBA32
    );
    if (!result) return nullptr;

    SDL_FillRect(result, nullptr, SDL_MapRGBA(result->format, 0, 0, 0, 0));

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

            uint32_t countryColor = SDL_MapRGB(result->format, c->color.r, c->color.g, c->color.b);

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
// surface → texture
// ===============================================================================================================
inline SDL_Texture* surfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface) {
    if (!surface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surface);
    return texture;
}
// optimization
inline void buildProvinceIdMap(World& world) {
    world.provinceById.clear();
    for (auto& province : world.provinces)
        world.provinceById[province.id] = &province;
}

// ===============================================================================================================
// LOAD ALL
// ===============================================================================================================
inline void loadAssets(World& world, SDL_Renderer* renderer) {
    auto start = std::chrono::high_resolution_clock::now();
    std::string folderpath = "assets/terrainCustom/";
    world.provincesBmp = IMG_Load((folderpath + "provinces.bmp").c_str());
    if (!world.provincesBmp) {
        std::cerr << "Error loading provinces.bmp: " << IMG_GetError() << "\n";
        return;
    }

    world.terrain = surfaceToTexture(renderer, IMG_Load((folderpath + "terrain.bmp").c_str()));
    world.height  = surfaceToTexture(renderer, IMG_Load((folderpath + "heightmap.bmp").c_str()));

    world.texWidth  = world.provincesBmp->w;
    world.texHeight = world.provincesBmp->h;

    world.provinces = loadProvinces(world, "assets/provinces.txt");
    world.countries = loadCountries();
    desaturateCountries(world.countries, 0.3, 5);
    world.armies    = loadArmies("assets/armies.txt", world);
    world.countriesImg = prepareCountries(renderer,world);
    world.countriesTex = surfaceToTexture(renderer, world.countriesImg);

    world.provinceFrontiers = findFrontiers(world.provincesBmp);
    world.countryFrontiers  = findFrontiersBetweenCountries(world, world.provinceFrontiers);

    world.adjacencyGraph = buildAdjacency(world.provinceFrontiers, world.provinces);
    buildProvinceIdMap(world);
    buildAccessibilityGraphsPerCountry(world);
    

    world.texStone = IMG_LoadTexture(renderer, "assets/ui/table.png");
    world.bootonTex = IMG_LoadTexture(renderer, "assets/ui/booton.png");
    world.statusBarTexture = IMG_LoadTexture(renderer, "assets/ui/statusBar.png");
    world.timeFrameTexture = IMG_LoadTexture(renderer, "assets/ui/timeFrame.png");
    world.flagFrameTexture = IMG_LoadTexture(renderer, "assets/ui/flagFrame.png");

    if (TTF_Init() == -1) {
        SDL_Log("TTF init error: %s", TTF_GetError());
        return;
    }
    
    world.fonts.push_back(initFont(renderer, "simple", "assets/fonts/Nunito/Nunito-VariableFont_wght.ttf", {0, 0, 0, 255}, 11));
    world.fonts.push_back(initFont(renderer, "fancy", "assets/fonts/Cinzel/Cinzel-VariableFont_wght.ttf", {220, 220, 220, 255}, 15));
    

    auto end = std::chrono::high_resolution_clock::now();
    float ms = std::chrono::duration<float, std::milli>(end - start).count();


    world.finalScale = std::min(1920.0f / world.texWidth,1080.0f / world.texHeight) * world.scale;

    // center starts over europe 
    world.offsetX = 0.57f * (1920 - world.texWidth * world.finalScale);
    world.offsetY = 0.22f * (1080 - world.texHeight * world.finalScale);

    std::cerr << "time: " << ms << " ms\n";
}
