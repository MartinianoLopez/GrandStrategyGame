#pragma once

//===============================

#include "../Model/World.hpp"
#include "../utils.hpp"
#include "../Simulation/Time.hpp"
#include "../Simulation/Diplomacy.hpp"
#include "../Simulation/Economy.hpp"
#include "SDL_render.h"
#include "../utils/Timer.hpp"
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

inline std::vector<std::string> getPeaceTreatySideParticipants(const World& world, bool playerSide) {
    for (const auto& war : world.activeWars) {
        const auto contains = [](const std::vector<std::string>& countries, const std::string& tag) {
            return std::find(countries.begin(), countries.end(), tag) != countries.end();
        };
        if (contains(war.attackerCountries, world.playerCountry) && contains(war.defenderCountries, world.peaceTreatyTarget))
            return playerSide ? war.attackerCountries : war.defenderCountries;
        if (contains(war.defenderCountries, world.playerCountry) && contains(war.attackerCountries, world.peaceTreatyTarget))
            return playerSide ? war.defenderCountries : war.attackerCountries;
    }
    return {};
}

inline std::string peaceTreatyParticipantText(const World& world, bool playerSide) {
    const auto participants = getPeaceTreatySideParticipants(world, playerSide);
    std::string text;
    for (const auto& tag : participants) {
        const Country* country = findCountryByTag(world.countries, tag);
        if (!text.empty()) text += ", ";
        text += country ? country->name : tag;
    }
    return text;
}

inline std::string peaceTreatyProvinceText(const World& world, const std::vector<int>& provinceIds) {
    if (provinceIds.empty()) return "None";
    std::string text;
    for (int provinceId : provinceIds) {
        const Province* province = findProvinceById(world.provinces, provinceId);
        if (!province) continue;
        if (!text.empty()) text += ", ";
        text += province->name;
    }
    return text.empty() ? "None" : text;
}

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
        Province* p = findProvinceById(w.provinces, w.selectedProvince);
        return p ? p->name : "NONE";
    };

    world.ui.hooks["selected_country"] = [](World& w) {
        Province* p = findProvinceById(w.provinces, w.selectedProvince);
        if (!p) return std::string("Select a Kingdom");
        Country* c = findCountryByTag(w.countries, p->owner);
        if (c) w.playerCountry = c->tag;
        return c ? c->name : std::string("Select a Kingdom");
    };

    world.ui.hooks["alliance_button_label"] = [](World& w) {
        return isAllied(w, w.playerCountry, w.selectedCountry) ? std::string("Break Alliance") : std::string("Offer Alliance");
    };

    world.ui.hooks["peace_treaty_title"] = [](World& w) {
        return "Peace Treaty";
    };

    world.ui.hooks["peace_treaty_player_participants"] = [](World& w) {
        return peaceTreatyParticipantText(w, true);
    };
    world.ui.hooks["peace_treaty_target_participants"] = [](World& w) {
        return peaceTreatyParticipantText(w, false);
    };
    world.ui.hooks["peace_treaty_demands"] = [](World& w) {
        return peaceTreatyProvinceText(w, w.peaceTreatyDemands);
    };
    world.ui.hooks["peace_treaty_offers"] = [](World& w) {
        return peaceTreatyProvinceText(w, w.peaceTreatyOffers);
    };
}

// ===============================================================================================================
// calls
// ===============================================================================================================

inline void clearPeaceTreatyMap(World& world) {
    SDL_DestroyTexture(world.peaceTreatyMap);
    world.peaceTreatyMap = nullptr;
    SDL_FreeSurface(world.peaceTreatyMapSurface);
    world.peaceTreatyMapSurface = nullptr;
    world.peaceTreatyBaseColors.clear();
    world.peaceTreatyRenderedSelections.clear();
    world.peaceTreatyPlayerColor = 0;
    world.peaceTreatySelectionColor = 0;
}

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
        pauseTime(w);
    };
    world.ui.actions["timeSpeed1"] = [](World& w) {
        setNormalTimeSpeed(w);
    };
    world.ui.actions["timeSpeed2"] = [](World& w) {
        w.time.speed = 5;
    };
    world.ui.actions["timeSpeed3"] = [](World& w) {
        w.time.speed = 10;
    };
    world.ui.actions["mapModeAccess"] = [](World& w) {
        if (w.mapMode != MapMode::ACCESS) {
            w.mapMode = MapMode::ACCESS;
        } else {
            w.mapMode = MapMode::NORMAL;
        };
    };
    world.ui.actions["mapModeTerrain"] = [](World& w) {
        if (w.mapMode != MapMode::TERRAIN) {
            w.mapMode = MapMode::TERRAIN;
        } else {
            w.mapMode = MapMode::NORMAL;
        };
    };
    world.ui.actions["mapModeDiplomacy"] = [](World& w) {
        if (w.mapMode != MapMode::DIPLOMATIC) {
            w.mapMode = MapMode::DIPLOMATIC;
        } else {
            w.mapMode = MapMode::NORMAL;
        };
    };
    world.ui.actions["recruit"] = [](World& w) {
        if(w.recruitInMass == true){
            w.recruitInMass = false;
        }else{
            w.recruitInMass = true; 
        }
        
        
    };
    world.ui.actions["declareWar"] = [](World& w) {
        declareWar(w, w.playerCountry, w.selectedCountry);
    };
    world.ui.actions["offerAlliance"] = [](World& w) {
        offerAlliance(w, w.playerCountry, w.selectedCountry);
    };
    world.ui.actions["toggleAlliance"] = [](World& w) {
        if (isAllied(w, w.playerCountry, w.selectedCountry)) {
            breakAlliance(w, w.playerCountry, w.selectedCountry);
        } else {
            offerAlliance(w, w.playerCountry, w.selectedCountry);
        }
    };
    world.ui.actions["openPeaceTreaty"] = [](World& w) {
        if (!isAtWar(w, w.playerCountry, w.selectedCountry) || w.playerCountry == w.selectedCountry) return;
        clearPeaceTreatyMap(w);
        w.peaceTreatyTarget = w.selectedCountry;
        w.peaceTreatyDemands.clear();
        w.peaceTreatyOffers.clear();
        w.peaceTreatyMode = PeaceTreatyMode::DEMAND;
        Country* player = findCountryByTag(w.countries, w.playerCountry);
        Country* target = findCountryByTag(w.countries, w.peaceTreatyTarget);
        w.ui.Textures["peacePlayerFlag"] = player ? player->flag : nullptr;
        w.ui.Textures["peaceTargetFlag"] = target ? target->flag : nullptr;
        w.ui.pressedElements.erase("peaceTreatyOfferModeBtn");
        w.ui.pressedElements.insert("peaceTreatyDemandModeBtn");
        w.peaceTreatyDraftOpen = true;
        pauseTime(w);
    };
    world.ui.actions["peaceTreatyDemandMode"] = [](World& w) {
        if (w.peaceTreatyMode == PeaceTreatyMode::DEMAND) return;
        w.peaceTreatyMode = PeaceTreatyMode::DEMAND;
        w.peaceTreatyOffers.clear();
        w.ui.pressedElements.erase("peaceTreatyOfferModeBtn");
        clearPeaceTreatyMap(w);
    };
    world.ui.actions["peaceTreatyOfferMode"] = [](World& w) {
        if (w.peaceTreatyMode == PeaceTreatyMode::OFFER) return;
        w.peaceTreatyMode = PeaceTreatyMode::OFFER;
        w.peaceTreatyDemands.clear();
        w.ui.pressedElements.erase("peaceTreatyDemandModeBtn");
        clearPeaceTreatyMap(w);
    };
    world.ui.actions["toggleTreatyProvince"] = [](World& w) {
        std::vector<int>& provinces = w.peaceTreatyMode == PeaceTreatyMode::DEMAND
            ? w.peaceTreatyDemands : w.peaceTreatyOffers;
        const bool valid = w.peaceTreatyMode == PeaceTreatyMode::DEMAND
            ? canDemandProvinceInPeaceTreaty(w, w.playerCountry, w.peaceTreatyTarget, w.selectedProvince)
            : canOfferProvinceInPeaceTreaty(w, w.playerCountry, w.selectedProvince);
        if (!valid) return;
        auto province = std::find(provinces.begin(), provinces.end(), w.selectedProvince);
        if (province == provinces.end()) provinces.push_back(w.selectedProvince);
        else provinces.erase(province);
    };
    world.ui.actions["sendPeaceTreaty"] = [](World& w) {
        if (offerPeaceTreaty(w, w.playerCountry, w.peaceTreatyTarget, w.peaceTreatyDemands, w.peaceTreatyOffers)) {
            w.peaceTreatyDraftOpen = false;
            clearPeaceTreatyMap(w);
            w.peaceTreatyTarget = "NONE";
            w.peaceTreatyDemands.clear();
            w.peaceTreatyOffers.clear();
            setNormalTimeSpeed(w);
        }
    };
    world.ui.actions["cancelPeaceTreaty"] = [](World& w) {
        w.peaceTreatyDraftOpen = false;
        clearPeaceTreatyMap(w);
        w.peaceTreatyTarget = "NONE";
        w.peaceTreatyDemands.clear();
        w.peaceTreatyOffers.clear();
    };
    world.ui.actions["investInProvince"] = [](World& w) {
        Invest(w, w.playerCountry, w.selectedProvince);
    };
    world.ui.actions["recruitInProvince"] = [](World& w) {
        recruitArmy(w, w.playerCountry, w.selectedProvince);
    };
    world.ui.actions["splitArmies"] = [](World& w) {
        splitArmies(w);
    };
}

inline void registerUiConditions(World& world) {
    world.ui.conditions["selected_country_is_selectable"] = [](World& w) {
        return findCountryByTag(w.countries, w.selectedCountry) != nullptr;
    };

    world.ui.conditions["selected_country_is_selected"] = [](World& w) {
        return !w.peaceTreatyDraftOpen && w.selectedCountry != "NONE";
    };

    world.ui.conditions["selected_country_is_other_country"] = [](World& w) {
         return !w.peaceTreatyDraftOpen &&
             w.selectedCountry != "NONE" &&
               w.selectedCountry != w.playerCountry;
    };
    world.ui.conditions["selected_country_is_self_country"] = [](World& w) {
         return !w.peaceTreatyDraftOpen &&
             w.selectedCountry != "NONE" &&
               w.selectedCountry == w.playerCountry;
    };

    world.ui.conditions["selected_country_is_not_an_enemy"] = [](World& w) {
        return !isAtWar(w, w.playerCountry, w.selectedCountry);
    };
    world.ui.conditions["selected_country_is_allied"] = [](World& w) {
        return isAllied(w, w.playerCountry, w.selectedCountry);
    };
    world.ui.conditions["selected_country_is_not_allied"] = [](World& w) {
        return !isAllied(w, w.playerCountry, w.selectedCountry);
    };
    world.ui.conditions["selected_country_can_accept_alliance"] = [](World& w) {
        return canOfferAlliance(w, w.playerCountry, w.selectedCountry);
    };
    world.ui.conditions["selected_country_can_offer_or_break_alliance"] = [](World& w) {
        if (w.peaceTreatyDraftOpen || w.selectedCountry == "NONE" || w.selectedCountry == w.playerCountry) return false;
        return !isAtWar(w, w.playerCountry, w.selectedCountry) &&
               (isAllied(w, w.playerCountry, w.selectedCountry) || canOfferAlliance(w, w.playerCountry, w.selectedCountry));
    };
    world.ui.conditions["peace_treaty_can_open"] = [](World& w) {
        return !w.peaceTreatyDraftOpen && w.selectedCountry != w.playerCountry &&
               isAtWar(w, w.playerCountry, w.selectedCountry);
    };
    world.ui.conditions["peace_treaty_draft_open"] = [](World& w) {
        return w.peaceTreatyDraftOpen;
    };
    world.ui.conditions["peace_treaty_has_demands"] = [](World& w) {
        return !w.peaceTreatyDemands.empty() || !w.peaceTreatyOffers.empty();
    };
}

// ===============================================================================================================
// reload flag 
// ===============================================================================================================

inline void reloadFlagTextures(World& world) {
    for (auto& component : world.ui.uiElements){
        if (component.name == "countryFlagTex") {
            component.texture = world.ui.Textures["selectedCountryFlagTex"];
        } else if (component.name == "peacePlayerFlag") {
            component.texture = world.ui.Textures["peacePlayerFlag"];
        } else if (component.name == "peaceTargetFlag") {
            component.texture = world.ui.Textures["peaceTargetFlag"];
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
    el.tooltip = e.value("tooltip", std::string{});
    el.hoverable = e.value("hoverable", false);
    el.toggle = e.value("toggle", false);
    el.hardtoggle = e.value("hardToggle", false);
    el.group = e.value("group", std::string("none"));
    el.visibleWhen = e.value("visibleWhen", std::vector<std::string>{});
    for (const std::string& condition : el.visibleWhen) {
        if (!world.ui.conditions.count(condition)) {
            SDL_LogError(
                SDL_LOG_CATEGORY_APPLICATION,
                "Unknown UI visibility condition '%s' on element '%s'",
                condition.c_str(),
                el.name.c_str()
            );
        }
    }

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

inline void createEmptyTexture(World& world){
    SDL_Renderer* renderer = world.renderer;
    // 1x1 transparent texture, used as a "no texture" placeholder
    SDL_Texture* empty = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 1, 1);
    SDL_SetTextureBlendMode(empty, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, empty);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);

    world.ui.Textures["empty"] = empty;
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

    createEmptyTexture(world);
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

    // reload layout components
    if (userMovedToOtherScreen || isTimeForUiReloading) {

        { Timer t("reloading textures"); 
            loadAllUITextures(world); 
        }

        parseLayout(world);
        world.lastPlace    = world.ui.place;
        world.lastUIReload = frameStart;
    }
}

inline void initUi(World& world) {
    uiInformation(world);
    registerActions(world);
    registerUiConditions(world);
    loadAllUITextures(world);
}