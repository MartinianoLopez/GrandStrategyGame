#pragma once
#include <string>
#include "World.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <iostream>
#include <SDL2/SDL_image.h>
#include "utils.hpp"
#include <chrono>

// ===============================================================================================================
// in game UI
// ===============================================================================================================
void buildGameUI(World& world, SDL_Renderer* renderer) {
    for (auto t : world.uiTextures) SDL_DestroyTexture(t);
    world.uiTextures.clear();
    world.uiElements.clear();

    std::string path = "assets/flags/" + world.playerCountry + ".tga";
    SDL_Texture* flagTex = IMG_LoadTexture(renderer, path.c_str());
    world.uiTextures.push_back(flagTex);

    // bandera
    world.uiElements.push_back({{0.01f, 0.01f, 0.06f, 0.08f}, flagTex, {0,0,0,0}, 1, nullptr, nullptr});

    // barra superior
    world.uiElements.push_back({{0.08f, 0.01f, 0.60f, 0.05f}, nullptr, {26,26,26,220}, 1, nullptr, nullptr});

    // dinero
    world.uiElements.push_back({{0.09f, 0.02f, 0.05f, 0.04f}, nullptr, {0,0,0,0}, 2, nullptr,
        [&world]() {
            Country* player = countryTagFind(world.countries, world.playerCountry);
            return player ? "$" + std::to_string(player->money) : "$0";
        }});

    // poblacion
    world.uiElements.push_back({{0.20f, 0.02f, 0.05f, 0.04f}, nullptr, {0,0,0,0}, 2, nullptr,
        [&world]() {
            return "POP: 1200";
        }});

    // ejercito
    world.uiElements.push_back({{0.31f, 0.02f, 0.05f, 0.04f}, nullptr, {0,0,0,0}, 2, nullptr,
        [&world]() {
            int count = 0;
            for (auto& a : world.armies)
                if (a.owner == world.playerCountry) count++;
            return "ARM: " + std::to_string(count);
        }});

    // fecha
    world.uiElements.push_back({{0.42f, 0.02f, 0.08f, 0.04f}, nullptr, {0,0,0,0}, 2, nullptr,
        [&world]() {
            return "1444 Jan 1"; // hardcoded por ahora
        }});

    // provincia seleccionada
    world.uiElements.push_back({{0.35f, 0.92f, 0.30f, 0.06f}, world.bootonTex, {26,26,26,200}, 1, nullptr, nullptr});
    world.uiElements.push_back({{0.36f, 0.93f, 0.28f, 0.04f}, nullptr, {0,0,0,0}, 2, nullptr,
        [&world]() {
            Province* p = provinceFindById(world.provinces, world.selectedProvince);
            return p ? p->name : "Ninguna";
        }});

    std::sort(world.uiElements.begin(), world.uiElements.end(),
        [](const UIElement& a, const UIElement& b){ return a.zOrder < b.zOrder; });
}



// ===============================================================================================================
// Menu
// ===============================================================================================================
void buildMainMenuUI(World& world, SDL_Renderer* renderer) {
    for (auto t : world.uiTextures) SDL_DestroyTexture(t);
    world.uiTextures.clear();
    world.uiElements.clear();

    // fondo oscuro semitransparente detras de los botones
    world.uiElements.push_back({{0.30f, 0.60f, 0.40f, 0.32f}, nullptr, {10,10,10,180}, 1, nullptr, nullptr});

    // boton PLAY
    world.uiElements.push_back({{0.35f, 0.63f, 0.30f, 0.07f}, world.bootonTex, {34,139,34,240}, 2,
        [&world]() { world.place = MenuPlace::CountrySelection; },
        []() { return "PLAY"; }});

    // boton LOAD
    world.uiElements.push_back({{0.35f, 0.73f, 0.30f, 0.07f}, world.bootonTex, {60,90,160,240}, 2,
        [&world]() { /* load logic */ },
        []() { return "LOAD"; }});

    // boton EXIT
    world.uiElements.push_back({{0.35f, 0.83f, 0.30f, 0.07f}, world.bootonTex, {160,40,40,240}, 2,
        [&world]() { world.running = false; },
        []() { return "EXIT"; }});

    std::sort(world.uiElements.begin(), world.uiElements.end(),
        [](const UIElement& a, const UIElement& b){ return a.zOrder < b.zOrder; });
}

void buildCountrySelectionUI(World& world, SDL_Renderer* renderer) {
    for (auto t : world.uiTextures) SDL_DestroyTexture(t);
    world.uiTextures.clear();
    world.uiElements.clear();

    // --- Panel fondo ---
    world.uiElements.push_back({
        {0.76f, 0.20f, 0.22f, 0.65f},
        world.texStone, {20,20,20,220}, 1,
        nullptr, nullptr
    });

    // --- Nombre del país (centrado, sin fondo) ---
    world.uiElements.push_back({
        {0.76f, 0.22f, 0.22f, 0.05f},
        nullptr, {0,0,0,0}, 2,
        nullptr,
        [&world]() -> std::string {
            Province* p = provinceFindById(world.provinces, world.selectedProvince);
            if (!p) return "Selecciona una provincia";
            Country* c = countryTagFind(world.countries, p->owner);
            if (c) world.playerCountry = c->tag;
            return c ? c->name : "Sin dueño";
        }
    });

    // --- Flag 
    world.uiElements.push_back({
        {0.77f, 0.28f, 0.20f, 0.22f},  
        world.flagTex, {30,30,30,180}, 2,
        nullptr, nullptr
    });

    // --- Botón JUGAR ---
    world.uiElements.push_back({
        {0.79f, 0.68f, 0.16f, 0.06f},
        world.bootonTex, {34,139,34,240}, 2,
        [&world]() {
            if (!world.playerCountry.empty())
                world.place = MenuPlace::InGame;
        },
        []() -> std::string { return "JUGAR"; }
    });

    // --- Botón VOLVER ---
    world.uiElements.push_back({
        {0.79f, 0.76f, 0.16f, 0.06f},
        world.bootonTex, {160,40,40,240}, 2,
        [&world]() { world.place = MenuPlace::MainMenu; },
        []() -> std::string { return "VOLVER"; }
    });

    std::sort(world.uiElements.begin(), world.uiElements.end(),
        [](const UIElement& a, const UIElement& b){ return a.zOrder < b.zOrder; });
}



void buildUI(World& world, SDL_Renderer* renderer) {
    switch (world.place) {
        case MenuPlace::MainMenu:         buildMainMenuUI(world, renderer);        break;
        case MenuPlace::CountrySelection: buildCountrySelectionUI(world, renderer); break;
        case MenuPlace::InGame:           buildGameUI(world, renderer);            break;
    }
}