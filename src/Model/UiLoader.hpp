#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <SDL2/SDL.h>
#include "../Model/World.hpp"
#include "../utils.hpp"
#include "../Simulation/Time.hpp"
#include "../Simulation/Diplomacy.hpp"
#include <filesystem>

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
            w.place = MenuPlace::InGame;
    };

    world.ui.actions["goto:MainMenu"] = [](World& w) {
        w.place = MenuPlace::MainMenu;
    };

    world.ui.actions["goto:CountrySelection"] = [](World& w) {
        w.place = MenuPlace::CountrySelection;
    };

    world.ui.actions["goto:LoadGame"] = [](World& w) {
        w.place = MenuPlace::LoadGame;
    };

    world.ui.actions["goto:InGame"] = [](World& w) {
        w.place = MenuPlace::InGame;
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
// Textures 
// ===============================================================================================================
inline void registerTextures(World& world) {
    world.ui.textures["SelectedCountryflagTexture"] = &world.selectedCountryFlagTex;
    world.ui.textures["flagTexture"]                = &world.flagTex;
    world.ui.textures["flagFrameTexture"]            = &world.flagFrameTexture;
    world.ui.textures["statusBarTexture"]            = &world.statusBarTexture;
    world.ui.textures["timeFrameTexture"]            = &world.timeFrameTexture;
    world.ui.textures["bootonTex"]                   = &world.bootonTex;
    world.ui.textures["texStone"]                    = &world.texStone;
    world.ui.textures["none"]                        = nullptr;
}

// ===============================================================================================================
// Init model
// ===============================================================================================================

inline void reloadFlagTextures(World& world) {
    for (auto& component : world.uiElements)
        if (component.name == "countryFlagTex") {
            component.texture = world.selectedCountryFlagTex;
        }
}

inline std::string screenName(MenuPlace place) {
    switch (place) {
        case MenuPlace::MainMenu:          return "MainMenu";
        case MenuPlace::CountrySelection:  return "CountrySelection";
        case MenuPlace::InGame:            return "InGame";
        case MenuPlace::LoadGame:          return "LoadGame";
    }
    return "";
}

// ===============================================================================================================
// Ui Layout 
// ===============================================================================================================

inline void loadUIFromFile(
    World& world,
    const std::string& path,
    SDL_Renderer* renderer
) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    // clear existing elements
    world.uiElements.clear();

    std::string targetScreen = screenName(world.place);
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

        if (token == "PANEL") {
            std::string name;
            int x, y, w, h, r, g, b, a, z;
            std::string texName;
            ss >> name >> x >> y >> w >> h >> texName >> r >> g >> b >> a >> z;

            SDL_Texture* tex = nullptr;
            if (world.ui.textures.count(texName) && world.ui.textures[texName])
                tex = *world.ui.textures[texName];

            world.uiElements.push_back({
                {x, y, w, h}, tex,
                {(Uint8)r,(Uint8)g,(Uint8)b,(Uint8)a},
                z, nullptr, nullptr, "fancy", name
            });

        } else if (token == "TEXT") {
            std::string name;
            int x, y, w, h, z;
            std::string font, endpointKey;
            ss >> name >> x >> y >> w >> h >> font >> z >> endpointKey;

            std::function<std::string()> getText = nullptr;
            if (world.ui.hooks.count(endpointKey)) {
                auto fn = world.ui.hooks[endpointKey];
                getText = [fn, &world]() { return fn(world); };
            }

            world.uiElements.push_back({
                {x, y, w, h}, nullptr, {0,0,0,0},
                z, nullptr, getText, font, name
            });

        } else if (token == "BUTTON") {
            std::string name;
            int x, y, w, h, r, g, b, a;
            std::string actionKey;
            ss >> name >> x >> y >> w >> h >> r >> g >> b >> a >> actionKey;

            std::string label;
            std::getline(ss, label);
            if (!label.empty() && label[0] == ' ') label = label.substr(1);

            std::function<void()> onClick = nullptr;
            if (world.ui.actions.count(actionKey)) {
                auto fn = world.ui.actions[actionKey];
                onClick = [fn, &world]() { fn(world); };
            }

            world.uiElements.push_back({
                {x, y, w, h}, world.bootonTex,
                {(Uint8)r,(Uint8)g,(Uint8)b,(Uint8)a},
                2, onClick, [label]() { return label; }, "simple", name
            });

        } else if (token == "GROUPBUTTON") {
            std::string name;
            int x, y, w, h;
            std::string texture, action, group;
            ss >> name >> x >> y >> w >> h >> texture >> action >> group;

            std::function<void()> onClick = nullptr;
            if (world.ui.actions.count(action)) {
                auto fn = world.ui.actions[action];
                onClick = [fn, &world]() { fn(world); };
            }

            world.uiElements.push_back({
                {x, y, w, h}, world.ui.Textures[texture],
                {}, 2, onClick, nullptr, "simple", name
            });
        }
        std::sort(
            world.uiElements.begin(),
            world.uiElements.end(),
            [](const UIElement& a, const UIElement& b) {
                return a.zOrder < b.zOrder;
            }
        );
    }
}
inline void loadAllUITextures(World& world, SDL_Renderer* renderer) {
    namespace fs = std::filesystem;
    
    for (const auto& entry : fs::recursive_directory_iterator("assets/ui")) {
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

inline void initRegistry(World& world) {
    uiInformation(world);
    registerActions(world);
    registerTextures(world);
    loadAllUITextures(world, world.renderer);
}