#pragma once
#include <string>
#include "../Model/World.hpp"
#include "Army.hpp"
#include "Taxes.hpp"

static constexpr int DAYS_IN_MONTH[] = {31,28,31,30,31,30,31,31,30,31,30,31};
static constexpr const char* months[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};

inline std::string dateToString(World& world) {
    return std::to_string(world.time.date.day) + ' ' + months[world.time.date.month - 1] + ' ' + std::to_string(world.time.date.year);
}

inline void onNewDay(World& world)   { 
        updateArmyMovement(world);
}

inline void onNewMonth(World& world) { 
        collectTaxes(world);
}


inline void addOneDay(World& world) {
    if (world.time.date.day == DAYS_IN_MONTH[world.time.date.month - 1]) {
        world.time.date.day = 1;
        world.time.date.month++;
        onNewMonth(world);
    } else {
        world.time.date.day++;
    }
    if (world.time.date.month == 13) {
        world.time.date.month = 1;
        world.time.date.year++;
    }
    onNewDay(world);
}


inline void tick(World& world, float dt) {
    world.time.accumulator += dt * world.time.speed;
    while (world.time.accumulator >= 1.0f) {
        world.time.accumulator -= 1.0f;
        addOneDay(world);
    }
}