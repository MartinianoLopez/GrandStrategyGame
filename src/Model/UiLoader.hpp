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
    // sorts the ui elements by the zIndex for rendering
    std::sort(
        world.ui.uiElements.begin(),
        world.ui.uiElements.end(),
        [](const UIElement& a, const UIElement& b) {
            return a.zOrder < b.zOrder;
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

inline SDL_Color parseColor(const json& j) {
    if (!j.contains("color")) return {0,0,0,0};
    auto c = j["color"];
    return {(Uint8)c[0], (Uint8)c[1], (Uint8)c[2], (Uint8)c[3]};
}

inline void parsePanel(World& world, const json& e) {
    SDL_Texture* tex = nullptr;
    if (world.ui.Textures.count(e["texture"]))
        tex = world.ui.Textures[e["texture"]];

    world.ui.uiElements.push_back({
        {e["x"], e["y"], e["w"], e["h"]}, tex,
        parseColor(e),
        e["zOrder"], nullptr, nullptr, "fancy", e["name"]
    });
}

inline void parseText(World& world, const json& e) {
    std::function<std::string()> getText = nullptr;
    std::string endpointKey = e["endpoint"];
    if (world.ui.hooks.count(endpointKey)) {
        auto fn = world.ui.hooks[endpointKey];
        getText = [fn, &world]() { return fn(world); };
    }

    world.ui.uiElements.push_back({
        {e["x"], e["y"], e["w"], e["h"]}, nullptr, {0,0,0,0},
        e["zOrder"], nullptr, getText, e["font"], e["name"]
    });
}

inline void parseButton(World& world, const json& e) {
    std::function<void()> onClick = nullptr;
    std::string actionKey = e["action"];
    if (world.ui.actions.count(actionKey)) {
        auto fn = world.ui.actions[actionKey];
        onClick = [fn, &world]() { fn(world); };
    }

    SDL_Texture* tex = nullptr;
    if (world.ui.Textures.count(e["texture"]))
        tex = world.ui.Textures[e["texture"]];

    std::string label = e.value("label", "");

    world.ui.uiElements.push_back({
        {e["x"], e["y"], e["w"], e["h"]}, tex,
        parseColor(e),
        2, onClick, [label]() { return label; }, "simple", e["name"]
    });
}

inline void parseGroupButton(World& world, const json& e) {
    std::function<void()> onClick = nullptr;
    std::string actionKey = e["action"];
    if (world.ui.actions.count(actionKey)) {
        auto fn = world.ui.actions[actionKey];
        onClick = [fn, &world]() { fn(world); };
    }

    SDL_Texture* tex = nullptr;
    if (world.ui.Textures.count(e["texture"]))
        tex = world.ui.Textures[e["texture"]];

    world.ui.uiElements.push_back({
        {e["x"], e["y"], e["w"], e["h"]}, tex,
        {}, 2, onClick, nullptr, "simple", e["name"]
    });
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

        for (auto& e : screen["elements"]) {
            std::string type = e["type"];
            if      (type == "PANEL")       parsePanel(world, e);
            else if (type == "TEXT")        parseText(world, e);
            else if (type == "BUTTON")      parseButton(world, e);
            else if (type == "GROUPBUTTON") parseGroupButton(world, e);
        }
        break;
    }

    sortUiElements(world);
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
    loadAllUITextures(world, world.renderer);
}