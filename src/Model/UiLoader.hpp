#pragma once

//===============================

#include "../Model/World.hpp"
#include "../utils.hpp"
#include "../Simulation/Time.hpp"
#include "../Simulation/Diplomacy.hpp"
#include <filesystem>

//===============================

#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <SDL2/SDL.h>

//===============================


// ===============================================================================================================
// Hooks 
// ===============================================================================================================
inline void uiInformation(World& world) {

    world.ui.hooks["player_money"] = [](World& w) {
        Country* p = findCountryByTag(w.countries, w.playerCountry);
        return p ? std::to_string(p->money) : "0";
    };

    world.ui.hooks["army_count"] = [](World& w) {
        int count = 0;
        for (auto& a : w.armies)
            if (a.owner == w.playerCountry) count++;
        return std::to_string(count);
    };

    world.ui.hooks["date"] = [](World& w) {
        return dateToString(w);
    };

    world.ui.hooks["selected_province"] = [](World& w) {
        Province* p = provinceFindById(w.provinces, w.selectedProvince);
        return p ? p->name : "None";
    };

    world.ui.hooks["selected_country"] = [](World& w) {
        Province* p = provinceFindById(w.provinces, w.selectedProvince);
        if (!p) return std::string("Select a Kingdom");
        Country* c = findCountryByTag(w.countries, p->owner);
        if (c) w.playerCountry = c->tag;
        return c ? c->name : std::string("Select a Kingdom");
    };
}
// ===============================================================================================================
// calls
// ===============================================================================================================
inline void registerActions(World& world) {

    world.ui.actions["exit"] = [](World& w) {
        w.running = false;
    };

    world.ui.actions["play_if_selected"] = [](World& w) {
        if (!w.playerCountry.empty())
            w.ui.place = MenuPlace::InGame;
    };

    world.ui.actions["goto:MainMenu"] = [](World& w) {
        w.ui.place = MenuPlace::MainMenu;
    };

    world.ui.actions["goto:CountrySelection"] = [](World& w) {
        w.ui.place = MenuPlace::CountrySelection;
    };

    world.ui.actions["goto:LoadGame"] = [](World& w) {
        w.ui.place = MenuPlace::LoadGame;
    };

    world.ui.actions["goto:InGame"] = [](World& w) {
        w.ui.place = MenuPlace::InGame;
    };

    world.ui.actions["timeSpeed0"] = [](World& w) {
        w.time.speed = 0;
    };
    world.ui.actions["timeSpeed1"] = [](World& w) {
        w.time.speed = 2;
    };
    world.ui.actions["timeSpeed2"] = [](World& w) {
        w.time.speed = 5;
    };
    world.ui.actions["timeSpeed3"] = [](World& w) {
        w.time.speed = 10;
    };
    world.ui.actions["mapModeAccess"] = [](World& w) {
        if (w.mapMode != "access") {
            w.mapMode = "access";
        } else {
            w.mapMode = "normal";
        };
    };
    world.ui.actions["mapModeTerrain"] = [](World& w) {
        if (w.mapMode != "terrain") {
            w.mapMode = "terrain";
        } else {
            w.mapMode = "normal";
        };
    };
    world.ui.actions["mapModeDiplomacy"] = [](World& w) {
        if (w.mapMode != "diplomatic") {
            w.mapMode = "diplomatic";
        } else {
            w.mapMode = "normal";
        };
    };
    world.ui.actions["recruit"] = [](World& w) {
        w.recruitOneUnit = true;
    };
    world.ui.actions["declareWar"] = [](World& w) {
        declareWar(w, w.playerCountry, w.selectedCountry);
    };
}
// ===============================================================================================================
// reload flag 
// ===============================================================================================================

inline void reloadFlagTextures(World& world) {
    for (auto& component : world.ui.uiElements){
        if (component.name == "countryFlagTex") {
            component.texture = world.ui.Textures["selectedCountryFlagTex"];
        }        
    }
}

// ===============================================================================================================
// Ui Layout 
// ===============================================================================================================
inline std::string screenName(MenuPlace place) {
    switch (place) {
        case MenuPlace::MainMenu:          return "MainMenu";
        case MenuPlace::CountrySelection:  return "CountrySelection";
        case MenuPlace::InGame:            return "InGame";
        case MenuPlace::LoadGame:          return "LoadGame";
    }
    return "";
}

inline void parsePanel(World& world, std::istringstream& ss) {
    std::string name;
    int x, y, w, h, r, g, b, a, z;
    std::string texName;
    ss >> name >> x >> y >> w >> h >> texName >> r >> g >> b >> a >> z;

    SDL_Texture* tex = nullptr;
    if (world.ui.Textures.count(texName))
        tex = world.ui.Textures[texName];

    world.ui.uiElements.push_back({
        {x, y, w, h}, tex,
        {(Uint8)r,(Uint8)g,(Uint8)b,(Uint8)a},
        z, nullptr, nullptr, "fancy", name
    });
}

inline void parseText(World& world, std::istringstream& ss) {
    std::string name;
    int x, y, w, h, z;
    std::string font, endpointKey;
    ss >> name >> x >> y >> w >> h >> font >> z >> endpointKey;

    std::function<std::string()> getText = nullptr;
    if (world.ui.hooks.count(endpointKey)) {
        auto fn = world.ui.hooks[endpointKey];
        getText = [fn, &world]() { return fn(world); };
    }

    world.ui.uiElements.push_back({
        {x, y, w, h}, nullptr, {0,0,0,0},
        z, nullptr, getText, font, name
    });
}

inline void parseButton(World& world, std::istringstream& ss) {
    std::string name;
    int x, y, w, h, r, g, b, a;
    std::string actionKey, texture;
    ss >> name >> x >> y >> w >> h >> r >> g >> b >> a >> actionKey >> texture;

    std::string label;
    std::getline(ss, label);
    if (!label.empty() && label[0] == ' ') label = label.substr(1);

    std::function<void()> onClick = nullptr;
    if (world.ui.actions.count(actionKey)) {
        auto fn = world.ui.actions[actionKey];
        onClick = [fn, &world]() { fn(world); };
    }

    SDL_Texture* tex = nullptr;
    if (world.ui.Textures.count(texture))
        tex = world.ui.Textures[texture];

    world.ui.uiElements.push_back({
        {x, y, w, h}, tex,
        {(Uint8)r,(Uint8)g,(Uint8)b,(Uint8)a},
        2, onClick, [label]() { return label; }, "simple", name
    });
}

inline void parseGroupButton(World& world, std::istringstream& ss) {
    std::string name;
    int x, y, w, h;
    std::string texture, action, group;
    ss >> name >> x >> y >> w >> h >> texture >> action >> group;

    std::function<void()> onClick = nullptr;
    if (world.ui.actions.count(action)) {
        auto fn = world.ui.actions[action];
        onClick = [fn, &world]() { fn(world); };
    }

    SDL_Texture* tex = nullptr;
    if (world.ui.Textures.count(texture))
        tex = world.ui.Textures[texture];

    world.ui.uiElements.push_back({
        {x, y, w, h}, tex,
        {}, 2, onClick, nullptr, "simple", name
    });
}

inline void sortUiElements(World& world) {
    std::sort(
        world.ui.uiElements.begin(),
        world.ui.uiElements.end(),
        [](const UIElement& a, const UIElement& b) {
            return a.zOrder < b.zOrder;
        }
    );
}

inline void loadUIFromFile(World& world, const std::string& path, SDL_Renderer* renderer) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    world.ui.uiElements.clear();

    std::string targetScreen = screenName(world.ui.place);
    bool inTarget = false;

    std::string line;
    while (std::getline(file, line)) {

        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "SCREEN") {
            std::string name;
            ss >> name;
            inTarget = (name == targetScreen);
            continue;
        }

        if (!inTarget) continue;

        if      (token == "PANEL")       parsePanel(world, ss);
        else if (token == "TEXT")        parseText(world, ss);
        else if (token == "BUTTON")      parseButton(world, ss);
        else if (token == "GROUPBUTTON") parseGroupButton(world, ss);

        sortUiElements(world);
    }
}

inline void loadAllUITextures(World& world, SDL_Renderer* renderer) {
    namespace fs = std::filesystem;
    
    for (const auto& entry : fs::recursive_directory_iterator("assets/ui/textures")) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            std::string key = entry.path().stem().string(); // filename sin extensión
            SDL_Texture* tex = IMG_LoadTexture(renderer, entry.path().string().c_str());
            if (tex) {
                world.ui.Textures[key] = tex;
                //std::cout << "Loaded: " << key << "\n";
            }
        }
    }
}

// ===============================================================================================================
// change of place in the ui
// ===============================================================================================================

inline void reloadUI(World& world, Uint32 frameStart){
    bool placeChanged = world.ui.place != world.lastPlace;
    bool timerFired   = (frameStart - world.lastUIReload) >= 2000;

    if (placeChanged || (world.debugging && timerFired)) {
        loadUIFromFile(world, "assets/ui/ui_layout.txt", world.renderer);
        world.lastPlace    = world.ui.place;
        world.lastUIReload = frameStart;
    }
}

inline void initUi(World& world) {
    uiInformation(world);
    registerActions(world);
    loadAllUITextures(world, world.renderer);
}