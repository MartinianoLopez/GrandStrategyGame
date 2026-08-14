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
#include <nlohmann/json.hpp>
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

inline void sortUiElements(World& world) {
    std::sort(
        world.ui.uiElements.begin(),
        world.ui.uiElements.end(),
        [](const UIElement& a, const UIElement& b) {
            return a.zIndex < b.zIndex;
        }
    );
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

using json = nlohmann::json;

inline void parseElement(World& world, const json& e) {
    SDL_Texture* tex = nullptr;
    if (e.contains("texture") && !e["texture"].is_null() && world.ui.Textures.count(e["texture"].get<std::string>()))
        tex = world.ui.Textures[e["texture"].get<std::string>()];

    std::function<void()> onClick = nullptr;
    if (e.contains("action") && !e["action"].is_null()) {
        std::string actionKey = e["action"].get<std::string>();
        if (world.ui.actions.count(actionKey)) {
            auto fn = world.ui.actions[actionKey];
            onClick = [fn, &world]() { fn(world); };
        }
    }

    std::function<std::string()> textProvider = nullptr;
    if (e.contains("label") && !e["label"].is_null()) {
        std::string label = e["label"].get<std::string>();
        textProvider = [label]() { return label; };
    } else if (e.contains("endpoint") && !e["endpoint"].is_null()) {
        std::string endpointKey = e["endpoint"].get<std::string>();
        if (world.ui.hooks.count(endpointKey)) {
            auto fn = world.ui.hooks[endpointKey];
            textProvider = [fn, &world]() { return fn(world); };
        }
    }

    SDL_FRect rect {
        e["x"].get<float>(), e["y"].get<float>(),
        e["w"].get<float>(), e["h"].get<float>()
    };

    UIElement el;
    el.name = e["name"].get<std::string>();
    el.zIndex = e.value("zIndex", 0);
    el.rect = rect;
    el.texture = tex;
    el.onClick = onClick;
    el.textProvider = textProvider;
    el.font = e.value("font", std::string("default"));
    el.hoverable = e.value("hoverable", false);
    el.toggle = e.value("toggle", false);
    el.pressed = false;

    world.ui.uiElements.push_back(std::move(el));
}

inline void parseLayout(World& world) {
    const std::string& path = "assets/ui/ui_layout.json";
    std::ifstream file(path);
    if (!file.is_open()) return;

    json data;
    file >> data;

    world.ui.uiElements.clear();

    std::string targetScreen = screenName(world.ui.place);

    for (auto& screen : data["screens"]) {
        if (screen["name"] != targetScreen) continue;

        for (auto& e : screen["elements"])
            parseElement(world, e);

        break;
    }

    sortUiElements(world);
}

inline void loadAllUITextures(World& world) {
    SDL_Renderer* renderer = world.renderer;
    namespace fs = std::filesystem;

    for (const auto& entry : fs::recursive_directory_iterator("assets/ui/textures")) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            std::string key = entry.path().stem().string(); // filename
            SDL_Texture* tex = IMG_LoadTexture(renderer, entry.path().string().c_str());
            if (tex) {
                world.ui.Textures[key] = tex;
            }
        }
    }

    // 1x1 transparent texture, used as a "no texture" placeholder
    SDL_Texture* empty = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1, 1);
    SDL_SetTextureBlendMode(empty, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, empty);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);

    world.ui.Textures["empty"] = empty;
}

// ===============================================================================================================
// change of place in the ui
// ===============================================================================================================

inline void reloadUI(World& world, Uint32 frameStart){
    
    // if the user moved to other place of the game
    bool userMovedToOtherScreen = world.ui.place != world.lastPlace;

    // hot reloading only on debugging mode
    bool isTimeForUiReloading = (frameStart - world.lastUIReload) >= world.HOT_RELOAD_WAIT_TIME;
    if(world.DEBUGGING_MODE == false){
        isTimeForUiReloading = false;
    }

    if (userMovedToOtherScreen || isTimeForUiReloading) {
        parseLayout(world);
        world.lastPlace    = world.ui.place;
        world.lastUIReload = frameStart;
    }
}

inline void initUi(World& world) {
    uiInformation(world);
    registerActions(world);
    loadAllUITextures(world);
}