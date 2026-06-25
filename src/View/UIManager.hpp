#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <SDL2/SDL.h>
#include "../Model/World.hpp"
#include "../utils.hpp"
#include "../Model/saves.hpp"
#include "../Simulation/Time.hpp"

// ===============================================================================================================
// REGISTRY
// ===============================================================================================================

struct UIRegistry {
    std::unordered_map<std::string, std::function<std::string(World&)>> informationPoints;
    std::unordered_map<std::string, std::function<void(World&)>>        actions;
    std::unordered_map<std::string, SDL_Texture**>                      textures;
};

// ===============================================================================================================
// Hooks
// ===============================================================================================================
inline void uiInformation(UIRegistry& ui, World& world) {

    ui.informationPoints["player_money"] = [](World& w) {
        Country* p = findCountryByTag(w.countries, w.playerCountry);
        return p ? std::to_string(p->money) : "0";
    };

    ui.informationPoints["army_count"] = [](World& w) {
        int count = 0;
        for (auto& a : w.armies)
            if (a.owner == w.playerCountry) count++;
        return std::to_string(count);
    };

    ui.informationPoints["date"] = [](World& world) {
        return dateToString(world);
    };

    ui.informationPoints["selected_province"] = [](World& w) {
        Province* p = provinceFindById(w.provinces, w.selectedProvince);
        return p ? p->name : "None";
    };

    ui.informationPoints["selected_country"] = [](World& w) {
        Province* p = provinceFindById(w.provinces, w.selectedProvince);
        if (!p) return std::string("Select your Kingdom");
        Country* c = findCountryByTag(w.countries, p->owner);
        if (c) w.playerCountry = c->tag;
        return c ? c->name : std::string("Select your Kingdom");
    };
}
// ===============================================================================================================
// calls
// ===============================================================================================================
inline void registerActions(UIRegistry& ui, World& world) {

    ui.actions["exit"] = [](World& w) {
        w.running = false;
    };

    ui.actions["save"] = [](World& w) {
        saveGame(w);
        refreshSaveFiles(w);
    };

    ui.actions["play_if_selected"] = [](World& w) {
        if (!w.playerCountry.empty())
            w.place = MenuPlace::InGame;
    };

    ui.actions["goto:MainMenu"] = [](World& w) {
        w.place = MenuPlace::MainMenu;
    };

    ui.actions["goto:CountrySelection"] = [](World& w) {
        w.place = MenuPlace::CountrySelection;
    };

    ui.actions["goto:LoadGame"] = [](World& w) {
        w.place = MenuPlace::LoadGame;
    };

    ui.actions["goto:InGame"] = [](World& w) {
        w.place = MenuPlace::InGame;
    };
    ui.actions["timeSpeed0"] = [](World& w) {
        w.time.speed = 0;
    };
    ui.actions["timeSpeed1"] = [](World& w) {
        w.time.speed = 2;
    };
    ui.actions["timeSpeed2"] = [](World& w) {
        w.time.speed = 5;
    };
    ui.actions["timeSpeed3"] = [](World& w) {
        w.time.speed = 10;
    };
    ui.actions["mapModeAccess"] = [](World& w) {
        if(w.mapMode != "access"){
            w.mapMode = "access";
        }else{
            w.mapMode = "normal";
        };
    };
    ui.actions["mapModeTerrain"] = [](World& w) {
        if(w.mapMode != "terrain"){
            w.mapMode = "terrain";
        }else{
            w.mapMode = "normal";
        };
        
    };
}
// ===============================================================================================================
// Textures
// ===============================================================================================================
inline void registerTextures(UIRegistry& reg, World& world) {
    //reg.textures["flagTexture"]      = &world.flagTexture;
    reg.textures["flagTex"]          = &world.flagTex;
    reg.textures["flagFrameTexture"] = &world.flagFrameTexture;
    reg.textures["statusBarTexture"] = &world.statusBarTexture;
    reg.textures["timeFrameTexture"] = &world.timeFrameTexture;
    reg.textures["bootonTex"]        = &world.bootonTex;
    reg.textures["texStone"]         = &world.texStone;
    reg.textures["none"]             = nullptr;
}
// ===============================================================================================================
// Init
// ===============================================================================================================
inline void initRegistry(UIRegistry& reg, World& world) {
    uiInformation(reg, world);
    registerActions(reg, world);
    registerTextures(reg, world);
}

// ===============================================================================================================
// Router
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

// ===============================================================================================================
// Register
// ===============================================================================================================
inline void loadUIFromFile(
    World& world,
    UIRegistry& reg,
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

        // ===============================================================================================================
        // Panel
        // ===============================================================================================================
        if (token == "PANEL") {
            int x, y, w, h, r, g, b, a, z;
            std::string texName;
            ss >> x >> y >> w >> h >> texName >> r >> g >> b >> a >> z;

            SDL_Texture* tex = nullptr;
            if (reg.textures.count(texName) && reg.textures[texName])
                tex = *reg.textures[texName];

            
            // push ----------------------------------------------------
            world.uiElements.push_back({
                {x, y, w, h},
                tex,
                {(Uint8)r,(Uint8)g,(Uint8)b,(Uint8)a},
                z,
                nullptr,
                nullptr,
                "fancy"
            });
        // ===============================================================================================================
        // Text
        // ===============================================================================================================
        } else if (token == "TEXT") {
            int x, y, w, h, z;
            std::string font, endpointKey;
            ss >> x >> y >> w >> h >> font >> z >> endpointKey;

            std::function<std::string()> getText = nullptr;
            if (reg.informationPoints.count(endpointKey)) {
                auto fn = reg.informationPoints[endpointKey];
                getText = [fn, &world]() { return fn(world); };
            }

            
            // push ----------------------------------------------------
            world.uiElements.push_back({
                {x, y, w, h},
                nullptr,
                {0,0,0,0},
                z,
                nullptr,
                getText,
                font
            });
        // ===============================================================================================================
        // Button
        // ===============================================================================================================
        } else if (token == "BUTTON") {
            int x, y, w, h, r, g, b, a;
            std::string actionKey;
            ss >> x >> y >> w >> h >> r >> g >> b >> a >> actionKey;

            // label es el resto de la linea
            std::string label;
            std::getline(ss, label);
            if (!label.empty() && label[0] == ' ') label = label.substr(1);

            std::function<void()> onClick = nullptr;
            if (reg.actions.count(actionKey)) {
                auto fn = reg.actions[actionKey];
                onClick = [fn, &world]() { fn(world); };
            }


            // push ----------------------------------------------------
            world.uiElements.push_back({
                {x, y, w, h},
                world.bootonTex,
                {(Uint8)r,(Uint8)g,(Uint8)b,(Uint8)a},
                2,
                onClick,
                [label]() { return label; },
                "simple"
            });
        }else if (token == "GROUPBUTTON") {
            // GROUPBUTTON 96 80 2 2 terrainButton mapModeTerrain MapModeBootons
            int x, y, w, h;
            std::string texture;
            std::string action;
            std::string group;
            ss >> x >> y >> w >> h >> texture >> action >> group;

            std::function<void()> onClick = nullptr;
            if (reg.actions.count(action)) {
                auto fn = reg.actions[action];
                onClick = [fn, &world]() { fn(world); };
            }
            // push ----------------------------------------------------
            world.uiElements.push_back({
                {x, y, w, h},
                world.Textures[texture],
                {},
                2,
                onClick,
                nullptr,
                "simple"
            });
        }
    }

    std::sort(
        world.uiElements.begin(),
        world.uiElements.end(),
        [](const UIElement& a, const UIElement& b) {
            return a.zOrder < b.zOrder;
        }
    );
}