#pragma once

#include <string>
#include <chrono>
#include <SDL2/SDL_image.h>
#include "World.hpp"
#include "utils.hpp"
#include "saves.hpp"

// ===============================================================================================================
// UI HELPERS
// ===============================================================================================================

inline void clearUI(World& world) {

    for (auto texture : world.uiTextures)
        SDL_DestroyTexture(texture);

    world.uiTextures.clear();
    world.uiElements.clear();
}

inline void sortUI(World& world) {

    std::sort(
        world.uiElements.begin(),
        world.uiElements.end(),
        [](const UIElement& a, const UIElement& b) {
            return a.zOrder < b.zOrder;
        }
    );
}

inline void addPanel(
    World& world,
    SDL_FRect rect,
    SDL_Texture* texture,
    SDL_Color color,
    int zOrder = 1
) {

    world.uiElements.push_back({
        rect,
        texture,
        color,
        zOrder,
        nullptr,
        nullptr,
        "fancy"
    });
}

inline void addText(
    World& world,
    SDL_FRect rect,
    int zOrder,
    std::function<std::string()> text,
    std::string font
) {

    world.uiElements.push_back({
        rect,
        nullptr,
        {0,0,0,0},
        zOrder,
        nullptr,
        text,
        font
    });
}

inline void addButton(
    World& world,
    SDL_FRect rect,
    SDL_Color color,
    std::function<void()> onClick,
    const std::string& text
) {

    world.uiElements.push_back({
        rect,
        world.bootonTex,
        color,
        2,
        onClick,
        [text]() { return text; },
        "simple"
    });
}

// ===============================================================================================================
// IN-GAME UI
// ===============================================================================================================
    
    /*
    ideal layout:
        addPanel(
        world,
        position {10, 10}
        size {100, 100},
        texture,
        color {0,0,0,0}
    );
    */

inline void buildGameUI(World& world, SDL_Renderer* renderer) {

    clearUI(world);

    // Load player flag
    std::string flagPath =
        "assets/flags/" +
        world.playerCountry +
        ".tga";

    SDL_Texture* flagTexture =
        IMG_LoadTexture(renderer, flagPath.c_str());

    world.uiTextures.push_back(flagTexture);

    // Player flag
    addPanel(
        world,
        {0.02f, 0.012f, 0.06f, 0.08f},
        flagTexture,
        {0,0,0,0}
    );

    addPanel(
        world,
        {0.00f, 0.0f, 0.1f, 0.1f},
        world.flagFrameTexture,
        {0,0,0,0}
    );

    // Top bar
    addPanel(
        world,
        {0.2f, 0.001f, 0.60f, 0.1f},
        world.statusBarTexture,
        {26,26,26,220}
    );

    // Money
    addText(
        world,
        {0.42f, 0.02f, 0.05f, 0.04f},
        2,
        [&world]() {

            Country* player =
                countryTagFind(
                    world.countries,
                    world.playerCountry
                );

            return player
                ? "$" + std::to_string(player->money)
                : "$0";
        },
        "fancy"
    );

    // Army count
    addText(
        world,
        {0.56f, 0.02f, 0.05f, 0.04f},
        2,
        [&world]() {

            int armyCount = 0;

            for (auto& army : world.armies)
                if (army.owner == world.playerCountry)
                    armyCount++;

            return  std::to_string(armyCount);
        },
        "fancy"
    );
    // Date
    addPanel(
        world,
        {0.90f, 0.02f, 0.08f, 0.04f},
        world.timeFrameTexture,
        {26,26,26,200}
    );
    addText(
        world,
        {0.90f, 0.02f, 0.08f, 0.04f},
        2,
        []() {
            return "1444 Jan 1";
        },
        "fancy"
    );

    // Selected province background
    addPanel(
        world,
        {0.35f, 0.92f, 0.30f, 0.06f},
        world.bootonTex,
        {26,26,26,200}
    );

    // Selected province name
    addText(
        world,
        {0.36f, 0.93f, 0.28f, 0.04f},
        2,
        [&world]() {

            Province* province =
                provinceFindById(
                    world.provinces,
                    world.selectedProvince
                );

            return province
                ? province->name
                : "None";
        },
        "simple"
    );
// Background panel
addPanel(
    world,
    {0.68f, 0.50f, 0.50f, 0.50f},  // esquina abajo derecha
    world.texStone,
    {0,0,0,0}
);

// Save button
addButton(
    world,
    {0.850f, 0.68f, 0.16f, 0.06f},  // desplazado dentro del panel
    {60,90,160,240},
    [&world]() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::string saveName = std::to_string(time);
        saveGame(world);
        refreshSaveFiles(world);
    },
    "SAVE GAME"
);

// Back button
addButton(
    world,
    {0.850f, 0.76f, 0.16f, 0.06f},  // debajo del save button
    {160,40,40,240},
    [&world]() {
        world.place = MenuPlace::MainMenu;
    },
    "BACK"
);

    sortUI(world);
}

// ===============================================================================================================
// MAIN MENU UI
// ===============================================================================================================

inline void buildMainMenuUI(World& world, SDL_Renderer* renderer) {

    clearUI(world);

    // Background panel
    addPanel(
        world,
        {0.25f, 0.50f, 0.50f, 0.50f},
        world.texStone,
        {0,0,0,0}
    );

    // Play button
    addButton(
        world,
        {0.35f, 0.63f, 0.30f, 0.07f},
        {34,139,34,240},
        [&world]() {
            world.place = MenuPlace::CountrySelection;
        },
        "PLAY"
    );

    // Load button
    addButton(
        world,
        {0.35f, 0.73f, 0.30f, 0.07f},
        {60,90,160,240},
        [&world]() {
            world.place = MenuPlace::LoadGame;
        },
        "LOAD"
    );

    // Exit button
    addButton(
        world,
        {0.35f, 0.83f, 0.30f, 0.07f},
        {160,40,40,240},
        [&world]() {
            world.running = false;
        },
        "EXIT"
    );

    sortUI(world);
}

// ===============================================================================================================
// LOAD GAME UI
// ===============================================================================================================
inline void buildLoadGameUI(
    World& world,
    SDL_Renderer* renderer
) {

    clearUI(world);

    refreshSaveFiles(world);

    // Background
    addPanel(
        world,
        {0.25f, 0.15f, 0.50f, 0.70f},
        world.texStone,
        {0,0,0,0}
    );

    float y = 0.20f;

    // Save list
    for (const std::string& save : world.saveFiles) {

        addButton(
            world,
            {0.30f, y, 0.40f, 0.06f},
            {34,139,34,240},

            [&world, save]() {

                world.selectedSave = save;

                loadGame(world, save);

                world.place = MenuPlace::InGame;
            },

            save
        );

        y += 0.08f;
    }

    // Back button
    addButton(
        world,
        {0.30f, 0.78f, 0.40f, 0.06f},
        {160,40,40,240},

        [&world]() {
            world.place = MenuPlace::MainMenu;
        },

        "BACK"
    );

    sortUI(world);
}

// ===============================================================================================================
// COUNTRY SELECTION UI
// ===============================================================================================================

inline void buildCountrySelectionUI(World& world, SDL_Renderer* renderer) {

    clearUI(world);

    // Right panel background
    addPanel(
        world,
        {0.76f, 0.20f, 0.22f, 0.65f},
        world.texStone,
        {0,0,0,0}
    );

    // Country name
    addText(
        world,
        {0.76f, 0.22f, 0.22f, 0.05f},
        2,
        [&world]() {

            Province* province =
                provinceFindById(
                    world.provinces,
                    world.selectedProvince
                );

            if (!province)
                return std::string("Select your Kingdom");

            Country* country =
                countryTagFind(
                    world.countries,
                    province->owner
                );

            if (country)
                world.playerCountry = country->tag;

            return country
                ? country->name
                : "Select your Kingdom";
        },
        "fancy"
    );

    // Country flag
    addPanel(
        world,
        {0.77f, 0.28f, 0.20f, 0.22f},
        world.flagTex,
        {0,0,0,0},
        2
    );

    // Play button
    addButton(
        world,
        {0.79f, 0.68f, 0.16f, 0.06f},
        {34,139,34,240},
        [&world]() {

            if (!world.playerCountry.empty())
                world.place = MenuPlace::InGame;
        },
        "PLAY"
    );

    // Back button
    addButton(
        world,
        {0.79f, 0.74f, 0.16f, 0.06f},
        {160,40,40,240},
        [&world]() {
            world.place = MenuPlace::MainMenu;
        },
        "BACK"
    );

    sortUI(world);
}

// ===============================================================================================================
// UI BUILDER
// ===============================================================================================================

inline void buildUI(World& world, SDL_Renderer* renderer) {

    switch (world.place) {

        case MenuPlace::MainMenu:
            buildMainMenuUI(world, renderer);
            break;

        case MenuPlace::CountrySelection:
            buildCountrySelectionUI(world, renderer);
            break;

        case MenuPlace::InGame:
            buildGameUI(world, renderer);
            break;

        case MenuPlace::LoadGame:
            buildLoadGameUI(world, renderer);
            break;
    }
}