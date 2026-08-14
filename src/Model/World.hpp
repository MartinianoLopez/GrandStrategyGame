#pragma once

//=========================

//=========================

#include "SDL_render.h"
#include <SDL2/SDL.h>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <utility>
#include <SDL2/SDL_ttf.h>
#include <functional>

//=========================

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

//=========================

enum class TypeOfRelation { 
    PEACE,
    WAR,
    ALLIANCE,
};

struct Relationship {
    std::string tag;
    enum TypeOfRelation typeOfRelation;

    Relationship(std::string tag, TypeOfRelation typeOfRelation)
        : tag(tag), typeOfRelation(typeOfRelation) {}
};

struct Country {
    std::string tag;
    std::string name;
    Color color;
    int money;
    std::vector<std::string> accessibleCountries;
    std::map<int, std::vector<int>> accessibilityGraph;
    SDL_Texture* flag;
    std::vector<Relationship> relationships;

    Country(std::string tag, std::string name, Color color, int money, SDL_Texture* flag)
        : tag(tag), name(name), color(color), money(money), accessibleCountries{tag, "NONE"}, flag(flag) {}

    void addRelationship(Relationship relationship){
        relationships.push_back(relationship);
    }
    void addAccesibleCountries(std::string tag){
        accessibleCountries.push_back(tag);
    }
    std::vector<Relationship> getWarRelations() {
        std::vector<Relationship> wars;
        for (const Relationship& r : relationships)
            if (r.typeOfRelation == TypeOfRelation::WAR)
                wars.push_back(r);
        return wars;
    }
};

struct ProvinceData {
    int id;
    SDL_Color color;
    std::string name;
    std::string owner;
};

struct Province {
    int id;
    std::string name;
    std::string owner;
    Color color;

    std::string controller;
    SDL_Point center;
    std::vector<std::pair<uint16_t, uint16_t>> shape;

    Province(int id, std::string name, std::string owner, Color color)
        : id(id), name(name), owner(owner), controller(""), color(color) {}
};

struct Army {
    int position; 
    std::vector<int> path;
    int movementStage;

    std::string name;
    std::string owner; 
    int power;
    Color color;

    Army(int position, std::string name, std::string owner, int power, Color color)
        : position(position), name(name), owner(owner), power(power), color(color), movementStage(0) {}
};

//==================================

struct Glyph {
    SDL_Texture* tex = nullptr;
    int w = 0;
    int h = 0;
};

struct Font {
    std::string id;
    Glyph glyphs[128];
};

//==================================

struct UIRect { int x, y, w, h; };

struct UIElement {
   
    UIRect boundsNorm;
    SDL_Texture* texture; // 4
    SDL_Color color;
    int zOrder;
    std::function<void()> onClick;
    std::function<std::string()> getText;
    std::string font = "simple";
    std::string name;
    
    SDL_FRect calculateBase(int w, int h) const {

        SDL_FRect base = { boundsNorm.x/100.f * w, boundsNorm.y/100.f * h,
                   boundsNorm.w/100.f * w, boundsNorm.h/100.f * h };

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
        SDL_FRect r = calculateBase(w, h);
        return x >= r.x && x < r.x + r.w &&
               y >= r.y && y < r.y + r.h;
    }
};

//================================================

struct Date { 
    int year, month, day;

    Date(int year, int month, int day)
        : year(year), month(month), day(day) {}
};

struct Time {
    Date date;
    float accumulator = 0.0f;
    float speed = 1.0f;

    Time(int year, int month, int day)
        : date(year, month, day) {}
};

//==================================

struct World;

enum class MenuPlace { 
    MainMenu, 
    CountrySelection, 
    InGame,
    LoadGame
};

struct Ui{
    std::unordered_map<std::string, std::function<std::string(World&)>> hooks;
    std::unordered_map<std::string, std::function<void(World&)>>        actions;
    std::map<std::string, SDL_Texture*> Textures;  
    MenuPlace place = MenuPlace::MainMenu;
    std::vector<UIElement> uiElements;
};

struct World{
    World(){
        window = SDL_CreateWindow(
            "Window", 
            SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, 
            1920, 1080, 
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );
        renderer = SDL_CreateRenderer(
            window, 
            -1, 
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    }
    //Dev Flags
    const bool DEBUGGING_MODE = true;
    const int HOT_RELOAD_WAIT_TIME = 200;

    bool running = true;
    

    Ui ui = Ui();
    MenuPlace lastPlace;

    SDL_Window* window;
    SDL_Renderer* renderer;

    Uint32 lastTicks    = 0;
    Uint32 lastUIReload = 0;

    Time time = Time(1444, 11, 1);
    int armyMovementSpeed = 25;
    
    SDL_Surface* provincesBmp = nullptr;
    SDL_Texture* terrain = nullptr;
    SDL_Texture* height = nullptr;
    SDL_Surface* countriesImg = nullptr;
    SDL_Texture* countriesTex = nullptr;
    SDL_Surface* controlSur = nullptr;
    SDL_Texture* controlTex = nullptr;
    
    std::vector<ProvinceData> provincesData;
    std::list<Province> provinces;
    std::list<Country> countries;
    std::list<Army> armies;

    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> provinceFrontiers;
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> countryFrontiers;
    std::map<int, std::vector<int>> adjacencyGraph;
    std::unordered_map<int, Province*> provinceById;

    int selectedProvince = 0;
    int objectiveProvince = 0;
    
    std::string selectedCountry = "";
    std::vector<Army*> selectedArmies;

    int texWidth = 0;
    int texHeight = 0;

    float scale = 5.0f;
    float finalScale = 0.0f; 

    float offsetX = 0.0f;
    float offsetY = 0.0f; 

    int lastX = 0;
    int lastY = 0;

    int frameDelay = 0;
    float fps = 0.0f;
   
    bool dragging = false;
    bool freecamera = false; 

    std::list<Font> fonts;
    
    // player
    std::string playerCountry;

    //====================================================================
    // Map modes
    //====================================================================
    
    std::string mapMode = "normal";
    SDL_Texture* activeAccessibilityMap = nullptr;
    SDL_Texture* activeDiplomaticMap = nullptr;
    std::string countryoftheAccesibilityMap = "";

    bool recruitOneUnit = false;
};

