#pragma once
#include <SDL2/SDL.h>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <utility>
#include <SDL2/SDL_ttf.h>

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

    Army(int position, std::string name, std::string owner, int power)
        : position(position), name(name), owner(owner), power(power) {}
    Army(int position, std::string owner, int power)
        : position(position), name("army"), owner(owner), power(power) {}
    Army(int position, int power)
        : position(position), name("Debuging Army"), owner("no Owner"), power(power) {}
};
struct Glyph {
    SDL_Texture* tex = nullptr;
    int w = 0;
    int h = 0;
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

    std::string playerCountry;

    TTF_Font* font = nullptr;
    Glyph glyphs[128];

    int texWidth = 0;
    int texHeight = 0;

    float scale = 2.0f;
    float offsetX = 0.f;
    float offsetY = 0.f;
    bool dragging = false;
    int lastX = 0, lastY = 0;
    int frameDelay = 1000 / 60;
    float fps = 0.0f;
    float finalScale = 0.f;
    bool running = true;
};

