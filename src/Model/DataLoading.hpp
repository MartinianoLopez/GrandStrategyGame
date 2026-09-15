#pragma once

// ========================

#include "SDL_stdinc.h"
#include "World.hpp"
#include "../utils.hpp"

// ========================

#include <cstdlib>
#include <string>
#include "SDL_render.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_image.h>

inline TerrainType parseTerrainType(const std::string& s) {
    if (s == "ocean")
        return TerrainType::OCEAN;
    return TerrainType::LAND; // default / "land"
}

inline void loadProvincesTxt(World& world) {
    std::ifstream file("assets/provinces.txt");

    if (!file.is_open()) {
        return;
    }

    std::string line;
    std::getline(file, line); // header

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> tokens;

        while (std::getline(ss, field, ';'))
            tokens.push_back(field);

        if (tokens.size() < 7) continue;

        try {
            ProvinceData pd;
            pd.id          = std::stoi(tokens[0]);
            pd.terrainType = parseTerrainType(tokens[1]);
            pd.color.r     = static_cast<uint8_t>(std::stoi(tokens[2]));
            pd.color.g     = static_cast<uint8_t>(std::stoi(tokens[3]));
            pd.color.b     = static_cast<uint8_t>(std::stoi(tokens[4]));
            pd.name        = tokens[5];
            pd.owner       = tokens[6];

            world.provincesData.push_back(pd);
        } catch (const std::exception&) {
            // invalid line, just ignores it
        }
    }
}


inline void loadCountries(World& world) {
    SDL_Renderer* renderer = world.renderer;

    std::ifstream file("assets/countries.txt");

    if (!file.is_open()) {
        std::cerr << "loadCountries Error: could not open countries.txt\n";
    }

    // format:
    // tag;name;r;g;b;money

    std::string line;

    // skip header
    std::getline(file, line);

    while (std::getline(file, line)) {

        std::stringstream ss(line);

        std::string token;

        std::vector<std::string> parts;

        while (std::getline(ss, token, ';')) {
            parts.push_back(token);
        }

        if (parts.size() < 6)
            continue;

        std::string tag = parts[0];
        std::string name = parts[1];

        Uint8 r = 0, g = 0, b = 0, money = 0;
        try {
            r = std::atoi(parts[2].c_str());
            g = std::atoi(parts[3].c_str());
            b = std::atoi(parts[4].c_str());
            money = std::atoi(parts[5].c_str());
        } catch (...) { continue; }

        std::string path = "assets/flags/" + tag + ".tga";
        SDL_Texture* flag = IMG_LoadTexture(renderer, path.c_str());
        world.countries.emplace_back(
            tag,
            name,
            SDL_Color{r, g, b, 255},
            money,
            flag
        );
    }
}

inline void loadArmies(World& world) {
    std::ifstream file("assets/armies.txt");

    if (!file.is_open()) {
        std::cerr << "loadArmies Error: could not open: armies.txt" << "\n";
    }

    std::string line;
    while (std::getline(file, line)) {

        std::istringstream ss(line);
        std::vector<std::string> parts;
        std::string token;

        while (std::getline(ss, token, ';'))
            parts.push_back(token);

        if (parts.size() < 4) continue;

        auto toInt = [](const std::string& s, int fallback = 0) -> int {
            try { return s.empty() ? fallback : std::atoi(s.c_str()); }
            catch (...) { return fallback; }
        };

        SDL_Color color = {0, 0, 0, 0};
        Country* c = findCountryByTag(world.countries, parts[2]);
        if (c) color = c->color;

        world.armies.emplace_back(
            toInt(parts[0]),
            parts[1],
            parts[2],
            toInt(parts[3]),
            color
        );
    }
}