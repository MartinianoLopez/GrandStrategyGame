#include "Loader.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <iostream>
#include <SDL2/SDL_image.h>
#include "utils.hpp"


// ===============================================================================================================
// LOAD ALL
// ===============================================================================================================

void loadAssets(GameData& state, SDL_Renderer* renderer) {

    state.provincesBmp = loadProvincesImage("assets/provinces.bmp");
    state.terrain = surfaceToTexture(renderer, IMG_Load("assets/terrain.bmp"));
    state.BmpColorToProvinceId = loadDefinitions("assets/definition.csv");
    state.ProvinceIdToCountryTag = loadProvincesFiles("assets/provinces");
    state.CountryTagToCountryName = loadCountryNames("assets/00_countries.txt");
    state.CountryNameToCountryColor = loadOwnerToColor("assets/countries");
    state.frontierList = findFrontiers(state.provincesBmp);
    
    state.countries = initCountries(state);
    state.texWidth = state.provincesBmp->w;
    state.texHeight = state.provincesBmp->h;
    state.frontierTexture = surfaceToTexture(renderer, createFrontiersSurface(state));
    /*
    showMap(state.BmpColorToProvinceId);
    showMap(state.ProvinceIdToCountryTag);
    showMap(state.CountryTagToCountryName);
    showMap(state.CountryNameToCountryColor);
    */
}


// ===============================================================================================================
// provinces.bmp
// ===============================================================================================================

SDL_Surface* loadProvincesImage(const std::string& filepath) {

    SDL_Surface* raw = IMG_Load(filepath.c_str());
    if (!raw) {
        std::cerr << "IMG_Load error: " << IMG_GetError() << "\n";
        return nullptr;
    }

    SDL_Surface* converted =
        SDL_ConvertSurfaceFormat(raw, SDL_PIXELFORMAT_RGBA32, 0);

    SDL_FreeSurface(raw);

    if (!converted) {
        std::cerr << "ConvertSurface error: " << SDL_GetError() << "\n";
        return nullptr;
    }

    return converted;
}


// ===============================================================================================================
// definitions.csv RGBColorFromProvinces.bmp ----> ProvinceID
// ===============================================================================================================

std::map<uint32_t, uint32_t>
loadDefinitions(const std::string& filepath) {
    std::map<uint32_t, uint32_t> colorMap;

    std::ifstream file(filepath);
    if (!file.is_open()) return colorMap;

    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string id, r, g, b, name, x;

        std::getline(ss, id, ';');
        std::getline(ss, r, ';');
        std::getline(ss, g, ';');
        std::getline(ss, b, ';');
        std::getline(ss, name, ';');
        std::getline(ss, x, ';');

        if (id.empty() || r.empty() || g.empty() || b.empty()) continue;

        uint32_t R = std::stoul(r);
        uint32_t G = std::stoul(g);
        uint32_t B = std::stoul(b);

        // formato RGBA32 (igual a getPixel con surface convertida)
        uint32_t color =
            (255u << 24) |  // A
            (B << 16)   |  // B
            (G << 8)    |  // G
            (R);           // R

        uint32_t provinceId = std::stoul(id);
        colorMap[color] = provinceId;
    }

    return colorMap;
}


// ===============================================================================================================
// provinces id-provinceName.txt    ProvinceID -----> CountryTag
// ===============================================================================================================

std::string extractOwnerFromLine(const std::string& line) {
    size_t eq = line.find('=');
    if (eq == std::string::npos) return "";
    
    std::string value = line.substr(eq + 1);
    
    // Limpiar espacios al inicio
    size_t start = value.find_first_not_of(" \t");
    if (start == std::string::npos) return "";
    
    value = value.substr(start);
    
    // Retornar solo los primeros 3 caracteres
    return value.substr(0, 3);
}

uint32_t extractProvinceId(const std::string& filename) {
    std::string idStr;
    for (char c : filename) {
        if (std::isdigit(c)) {
            idStr += c;
        } else {
            break;
        }
    }
    
    if (idStr.empty()) return 0;
    
    try {
        return static_cast<uint32_t>(std::stoul(idStr));
    } catch (...) {
        return 0;
    }
}

std::map<uint32_t, std::string> loadProvincesFiles(const std::string& dirpath) {
    std::map<uint32_t, std::string> idToOwner;
    namespace fs = std::filesystem;
    
    if (!fs::exists(dirpath)) {
        return idToOwner;
    }
    
    for (const auto& entry : fs::directory_iterator(dirpath)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        
        std::string filename = entry.path().filename().string();
        uint32_t id = extractProvinceId(filename);
        
        if (id == 0) {
            continue;
        }
        
        std::ifstream file(entry.path());
        if (!file.is_open()) {
            continue;
        }
        
        std::string line;
        bool found = false;
        
        while (std::getline(file, line) && !found) {
            // Si encontramos una fecha (ej: 1487.1.1 =), paramos
            // Las fechas tienen formato: YYYY.M.D =
            if (line.find_first_of("0123456789") == 0 && line.find('=') != std::string::npos) {
                // Es una fecha, no seguimos buscando
                break;
            }
            
            // Ignorar tribal_owner
            if (line.find("tribal_owner") != std::string::npos) {
                continue;
            }
            
            // Buscar "owner =" (sin espacios antes)
            size_t ownerPos = line.find("owner =");
            if (ownerPos != std::string::npos) {
                std::string owner = extractOwnerFromLine(line);
                if (!owner.empty()) {
                    idToOwner[id] = owner;
                    found = true; // Encontró, no seguir buscando
                }
            }
        }
    }
    
    return idToOwner;
}
// ===============================================================================================================
// 00_countries.txt        CountryTag ---> CountryName 
// ===============================================================================================================
std::map<std::string, std::string> loadCountryNames(const std::string& filepath) {
    std::map<std::string, std::string> tagToName;
    std::ifstream file(filepath);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty())                continue;
        if (line[0] == '#')              continue;  // ignorar comentarios
        
        auto eq = line.find('=');
        if (eq == std::string::npos)     continue;

        std::string tag  = line.substr(0, eq);
        std::string path = line.substr(eq + 1);

        tag.erase(0, tag.find_first_not_of(" \t"));
        tag.erase(tag.find_last_not_of(" \t") + 1);
        path.erase(0, path.find_first_not_of(" \t\""));
        path.erase(path.find_last_not_of(" \t\"") + 1);

        // "countries/Sweden.txt" ---> "Sweden"
        auto slash = path.find_last_of('/');
        auto dot   = path.find_last_of('.');
        std::string name = path.substr(slash + 1, dot - slash - 1);

        tagToName[tag] = name;  // SWE -> Sweden
    }
    return tagToName;
}

// ===============================================================================================================
// countries/country.txt   CountryName ---> ColorRGB 
// ===============================================================================================================
std::map<std::string, uint32_t> loadOwnerToColor(const std::string& dirpath) {
    std::map<std::string, uint32_t> nameToColor;

    for (const auto& entry : std::filesystem::directory_iterator(dirpath)) {
        std::ifstream file(entry.path());
        std::string line;
        std::string filename = entry.path().stem().string();

        while (std::getline(file, line)) {
            if (line.find("color") == std::string::npos) continue;
            if (line.find('{')    == std::string::npos) continue;

            std::istringstream iss(line.substr(line.find('{') + 1));
            uint32_t r, g, b;
            iss >> r >> g >> b;

            nameToColor[filename] = (r << 16) | (g << 8) | b;
            break;
        }
    }
    return nameToColor;
}


// ===============================================================================================================
// frontiers
// ===============================================================================================================

    // Frontier ordering issues:
    //
    // Main problem: frontier points are NOT ordered.
    // Current extraction gives scattered pixels instead of a continuous border.
    //
    // Example (unordered points):
    //
    //        1  2  3  4  5  6        ---------> line by line reading
    //   7  8                   9 10  --------->
    //
    // Desired (ordered path):
    //
    //        3  4  5  6  7  8
    //   1  2                  9 10
    //
    // Possible solutions:
    // - Sort points into a continuous path (costly one time only)
    // - Use a diferent boundry finder (better but complex)

std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_Point>>
findFrontiers(SDL_Surface* img) {
    int imgW = img->w;
    int imgH = img->h;
    
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_Point>> frontierList;
    
    for (int y = 0; y < imgH; y++) {
        for (int x = 0; x < imgW; x++) {
            uint32_t current = getPixelColor(img, x, y);
            bool isFrontier = false;
            uint32_t neighborColor = current;
            
            // Derecha
            if (x + 1 < imgW) {
                uint32_t right = getPixelColor(img, x + 1, y);
                if (right != current) {
                    isFrontier = true;
                    neighborColor = right;
                }
            }
            
            // Abajo
            if (!isFrontier && y + 1 < imgH) {
                uint32_t below = getPixelColor(img, x, y + 1);
                if (below != current) {
                    isFrontier = true;
                    neighborColor = below;
                }
            }
            
            // Diagonal
            if (!isFrontier && x + 1 < imgW && y + 1 < imgH) {
                uint32_t diag = getPixelColor(img, x + 1, y + 1);
                if (diag != current) {
                    isFrontier = true;
                    neighborColor = diag;
                }
            }
            
            if (isFrontier) {
                frontierList[{current, neighborColor}].push_back({x, y});
            }
        }
    }
    
    return frontierList;
}

SDL_Surface* createFrontiersSurface(GameData& state) {
    std::cerr << "Frontiers surface: " << state.texWidth << "x" << state.texHeight << " points: " << state.frontierList.size() << "\n";
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, state.texWidth, state.texHeight, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 0, 0, 0, 0));

    for (const auto& [colorPair, points] : state.frontierList)
        for (const auto& point : points)
            if (point.x >= 0 && point.x < state.texWidth && point.y >= 0 && point.y < state.texHeight)
    ((Uint32*)surface->pixels)[point.y * state.texWidth + point.x] = SDL_MapRGBA(surface->format, 0, 0, 0, 255);

    return surface;
}


// ===============================================================================================================
// Utils
// ===============================================================================================================
void setPixel(SDL_Surface* surface, int x, int y, uint32_t color) {
    uint8_t r = color & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = (color >> 16) & 0xFF;
    uint8_t a = (255);
    Uint32 pixel = SDL_MapRGBA(
        surface->format,
        b, g, r, a
    );

    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * 4;
    *(Uint32*)p = pixel;
}

SDL_Surface* initCountries(const GameData& state) {
    SDL_Surface* provinces = state.provincesBmp;
    if (!provinces) return nullptr;

    SDL_Surface* countries = SDL_CreateRGBSurfaceWithFormat(
        0,
        provinces->w,
        provinces->h,
        32,
        provinces->format->format
    );
    if (!countries) return nullptr;

    SDL_LockSurface(provinces);
    SDL_LockSurface(countries);

    int imgW = provinces->w;
    int imgH = provinces->h;

    for (int y = 0; y < imgH; y++) {
        for (int x = 0; x < imgW; x++) {
            uint32_t pixel = getPixelColor(provinces, x, y);
            uint32_t country = getCountryColorFromProvinceColor(state, pixel);
            if (country != 0) {
                setPixel(countries, x, y, country);
            }
        }
    }

    SDL_UnlockSurface(provinces);
    SDL_UnlockSurface(countries);
    return countries;
}