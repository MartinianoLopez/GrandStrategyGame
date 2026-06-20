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

// ===============================================================================================================
// REGISTRY
// ===============================================================================================================

struct UIRegistry {
    std::unordered_map<std::string, std::function<std::string(World&)>> endpoints;
    std::unordered_map<std::string, std::function<void(World&)>>        actions;
    std::unordered_map<std::string, SDL_Texture**>                      textures;
};

// ===============================================================================================================
// Hooks
// ===============================================================================================================
inline void registerEndpoints(UIRegistry& reg, World& world) {

    reg.endpoints["player_money"] = [](World& w) {
        Country* p = countryTagFind(w.countries, w.playerCountry);
        return p ? "$" + std::to_string(p->money) : "$0";
    };

    reg.endpoints["army_count"] = [](World& w) {
        int count = 0;
        for (auto& a : w.armies)
            if (a.owner == w.playerCountry) count++;
        return std::to_string(count);
    };

    reg.endpoints["date"] = [](World& w) {
        return w.date;
    };

    reg.endpoints["selected_province"] = [](World& w) {
        Province* p = provinceFindById(w.provinces, w.selectedProvince);
        return p ? p->name : "None";
    };

    reg.endpoints["selected_country"] = [](World& w) {
        Province* p = provinceFindById(w.provinces, w.selectedProvince);
        if (!p) return std::string("Select your Kingdom");
        Country* c = countryTagFind(w.countries, p->owner);
        if (c) w.playerCountry = c->tag;
        return c ? c->name : std::string("Select your Kingdom");
    };
}
// ===============================================================================================================
// calls
// ===============================================================================================================
inline void registerActions(UIRegistry& reg, World& world) {

    reg.actions["exit"] = [](World& w) {
        w.running = false;
    };

    reg.actions["save"] = [](World& w) {
        saveGame(w);
        refreshSaveFiles(w);
    };

    reg.actions["play_if_selected"] = [](World& w) {
        if (!w.playerCountry.empty())
            w.place = MenuPlace::InGame;
    };

    reg.actions["goto:MainMenu"] = [](World& w) {
        w.place = MenuPlace::MainMenu;
    };

    reg.actions["goto:CountrySelection"] = [](World& w) {
        w.place = MenuPlace::CountrySelection;
    };

    reg.actions["goto:LoadGame"] = [](World& w) {
        w.place = MenuPlace::LoadGame;
    };

    reg.actions["goto:InGame"] = [](World& w) {
        w.place = MenuPlace::InGame;
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
    registerEndpoints(reg, world);
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
            if (reg.endpoints.count(endpointKey)) {
                auto fn = reg.endpoints[endpointKey];
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