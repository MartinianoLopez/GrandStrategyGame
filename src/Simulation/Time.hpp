#pragma once
#include "../Model/World.hpp"
#include "Army.hpp"

inline void updateWorld(World& world){
    updateArmyMovement(world);
}

inline void timeRun(World& world, float deltaTime) {
    world.timeAccumulator += deltaTime * world.timeSpeed;
    if (world.timeAccumulator >= 1.0f) {
        world.timeAccumulator -= 1.0f;
        world.days += 1;
        updateWorld(world);
    }
}