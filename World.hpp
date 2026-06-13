#pragma once
#include <SDL2/SDL.h>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <utility>
#include <SDL2/SDL_ttf.h>
#include <functional>
struct Color {
    int r;
    int g;
    int b;
    int a;

    Color(int r, int g, int b, int a)
        : r(r), g(g), b(b), a(a) {}

    Color(int r, int g, int b)
        : r(r), g(g), b(b), a(255) {}

    bool operator==(const Color& other) const {
        return r == other.r &&
               g == other.g &&
               b == other.b &&
               a == other.a;
    }

    bool operator!=(const Color& other) const {
        return !(*this == other);
    }
};

struct Country {
    std::string tag;
    std::string name;
    Color color;
    int money;

    Country(std::string tag, std::string name, Color color)
        : tag(tag), name(name), color(color), money(100) {}
    Country(std::string tag, std::string name, Color color, int money)
        : tag(tag), name(name), color(color), money(money) {}
};

struct Province {
    int id;
    std::string name;
    std::string owner;
    Color color;
    SDL_Point center;

    Province(int id, std::string name, std::string owner, Color color)
        : id(id), name(name), owner(owner), color(color) {}
};

struct Army {
    int position; // province Id
    std::string name;
    std::string owner; // country tag
    int power;
    Color color;

    Army(int position, std::string name, std::string owner, int power, Color color)
        : position(position), name(name), owner(owner), power(power), color(color) {}
};
struct Glyph {
    SDL_Texture* tex = nullptr;
    int w = 0;
    int h = 0;
};

struct UIElement {
    SDL_FRect boundsNorm;
    SDL_Texture* texture;
    SDL_Color color;
    int zOrder;
    std::function<void()> onClick;
    std::function<std::string()> getText;

    SDL_FRect resolve(int w, int h) const {
    SDL_FRect base = { boundsNorm.x * w, boundsNorm.y * h,
                       boundsNorm.w * w, boundsNorm.h * h };

    if (!texture) return base;

    int texW, texH;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);

    float texRatio  = (float)texW / texH;
    float boxRatio  = base.w / base.h;

    if (texRatio > boxRatio) {
        // limitado por ancho
        float newH = base.w / texRatio;
        base.y += (base.h - newH) * 0.5f;
        base.h  = newH;
    } else {
        // limitado por alto
        float newW = base.h * texRatio;
        base.x += (base.w - newW) * 0.5f;
        base.w  = newW;
    }

    return base;
}

    bool contains(int x, int y, int w, int h) const {
        SDL_FRect r = resolve(w, h);
        return x >= r.x && x < r.x + r.w &&
               y >= r.y && y < r.y + r.h;
    }
};

enum class MenuPlace { 
    MainMenu, 
    CountrySelection, 
    InGame,
    LoadGame
};

struct World {
    SDL_Surface* provincesBmp = nullptr;
    SDL_Texture* terrain = nullptr;
    SDL_Texture* height = nullptr;
    SDL_Surface* countriesImg = nullptr;
    
    std::list<Province> provinces;
    std::list<Country> countries;
    std::list<Army> armies;

    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> provinceFrontiers;
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> countryFrontiers;
    
    int selectedProvince = 0;
    int objectiveProvince = 0;

 

    TTF_Font* font = nullptr;
    Glyph glyphs[128];

    int texWidth = 0;
    int texHeight = 0;

    float scale = 5.0f;
    float offsetX = 200.0f; // broken -inf, -inf offset
    float offsetY = -200.0f; // broken -inf, -inf offset
    bool dragging = false;
    int lastX = 0, lastY = 0;
    int frameDelay = 1000 / 60;
    float fps = 0.0f;
    float finalScale = 0.f;
    bool running = true;
    bool freecamera = false; // broken -inf, -inf offset   
    
    std::string playerCountry;
    SDL_Texture* flagTex = nullptr;
    SDL_Texture* bootonTex = nullptr;
    MenuPlace place = MenuPlace::MainMenu;
    std::vector<SDL_Texture*> uiTextures;
    std::vector<UIElement>    uiElements;
    SDL_Texture* texStone = nullptr;
    SDL_Texture* statusBarTexture = nullptr;
    std::vector<std::string> saveFiles;
    std::string selectedSave;

};

