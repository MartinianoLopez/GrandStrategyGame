#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>
#include <algorithm>

#include "World.hpp"

namespace fs = std::filesystem;

// ===============================================================================================================
// SAVE SYSTEM
// ===============================================================================================================

const fs::path SAVE_FOLDER =
    fs::current_path() / "saves";

// ===============================================================================================================
// CREATE SAVE FOLDER
// ===============================================================================================================

inline void createSaveFolder() {

    try {

        if (!fs::exists(SAVE_FOLDER)) {

            fs::create_directories(SAVE_FOLDER);

            std::cout
                << "[SAVE] Save folder created"
                << std::endl;
        }
    }
    catch (const std::exception& e) {

        std::cerr
            << "[SAVE ERROR] "
            << e.what()
            << std::endl;
    }
}

// ===============================================================================================================
// GENERATE SAVE NAME
// Example:
// FRA_2026_05_24_22_41
// ===============================================================================================================

inline std::string generateSaveName(
    const World& world
) {

    auto now =
        std::chrono::system_clock::now();

    std::time_t time =
        std::chrono::system_clock::to_time_t(now);

    std::tm* localTime =
        std::localtime(&time);

    std::stringstream ss;

    ss
        << world.playerCountry
        << "_"

        << (1900 + localTime->tm_year)
        << "_";

    // Month
    if ((localTime->tm_mon + 1) < 10)
        ss << "0";

    ss
        << (localTime->tm_mon + 1)
        << "_";

    return ss.str();
}

// ===============================================================================================================
// CREATE SAVE DIRECTORY
// ===============================================================================================================

inline fs::path createSaveDirectory(
    const std::string& saveName
) {

    fs::path savePath =
        SAVE_FOLDER / saveName;

    try {

        if (!fs::exists(savePath)) {

            fs::create_directories(savePath);

            std::cout
                << "[SAVE] Save directory created"
                << std::endl;
        }
    }
    catch (const std::exception& e) {

        std::cerr
            << "[SAVE ERROR] "
            << e.what()
            << std::endl;
    }

    return savePath;
}

// ===============================================================================================================
// SAVE PROVINCES
// ===============================================================================================================

inline void saveProvinces(
    const World& world,
    const fs::path& savePath
) {

    fs::path path =
        savePath / "provinces.txt";

    std::ofstream file(path);

    if (!file.is_open()) {

        std::cerr
            << "[SAVE ERROR] Could not open provinces.txt"
            << std::endl;

        return;
    }

    for (const Province& province : world.provinces) {

        file
            << province.id << ";"
            << province.color.r << ";"
            << province.color.g << ";"
            << province.color.b << ";"
            << province.name << ";"
            << province.owner
            << "\n";
    }

    file.close();

    std::cout
        << "[SAVE] Provinces saved"
        << std::endl;
}

// ===============================================================================================================
// SAVE ARMIES
// ===============================================================================================================

inline void saveArmies(
    const World& world,
    const fs::path& savePath
) {

    fs::path path =
        savePath / "armies.txt";

    std::ofstream file(path);

    if (!file.is_open()) {

        std::cerr
            << "[SAVE ERROR] Could not open armies.txt"
            << std::endl;

        return;
    }

    for (const Army& army : world.armies) {

        file
            << army.position << ";"
            << army.name << ";"
            << army.owner << ";"
            << army.power
            << "\n";
    }

    file.close();

    std::cout
        << "[SAVE] Armies saved"
        << std::endl;
}

// ===============================================================================================================
// SAVE GAME
// ===============================================================================================================

inline void saveGame(
    const World& world
) {

    createSaveFolder();
    std::string saveName = generateSaveName(world);
    fs::path savePath =
        createSaveDirectory(saveName);

    saveProvinces(world, savePath);
    saveArmies(world, savePath);

    std::cout
        << "[SAVE] Game saved successfully"
        << std::endl;
}

// ===============================================================================================================
// REFRESH SAVE FILES
// ===============================================================================================================

inline void refreshSaveFiles(World& world) {

    world.saveFiles.clear();

    if (!fs::exists(SAVE_FOLDER))
        return;

    for (const auto& entry : fs::directory_iterator(SAVE_FOLDER)) {

        if (!entry.is_directory())
            continue;

        std::string saveName =
            entry.path().filename().string();

        world.saveFiles.push_back(saveName);
    }

    std::sort(
        world.saveFiles.begin(),
        world.saveFiles.end(),
        std::greater<std::string>()
    );
}

// ===============================================================================================================
// LOAD GAME
// ===============================================================================================================
inline void loadGame(
    World& world,
    const std::string& saveName
) {
    fs::path savePath = SAVE_FOLDER / saveName;

    if (saveName.size() >= 3)
        world.playerCountry = saveName.substr(0, 3);

    world.provinces = loadProvinces(world, (savePath / "provinces.txt").string());
    world.armies    = loadArmies((savePath / "armies.txt").string(), world);

    std::cout << "[LOAD] " << saveName << "\n";
}