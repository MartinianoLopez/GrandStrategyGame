#pragma once

//=============================

#include "../Model/World.hpp"
#include "../utils.hpp"
#include "ArmyRenderer.hpp"
#include "FrontierRenderer.hpp"
#include "MapModeRenderers.hpp"

//=============================

#include "SDL_rect.h"

//=============================

inline void renderMapModeLayer(World &world) {

  switch (world.mapMode) {
  case MapMode::NORMAL:
    renderNormalMap(world);
    break;

  case MapMode::ACCESS:
    renderAccessMap(world);
    break;

  case MapMode::DIPLOMATIC:
    renderDiplomaticMap(world);
    break;

  case MapMode::TERRAIN:
    break;

  default:
    break;
  }
}

inline void renderFrontiers(World &world) {

  if (world.scale > 6.0f)
    renderFrontiersAsPoints(world, {0, 0, 0, 120}, world.provinceFrontiers, 1);

  if (world.scale < 5.0f)
    renderFrontiersAsPoints(world, {0, 0, 0, 220}, world.countryFrontiers,
                            6 / world.scale);

  if (world.scale > 4.0f) {
    renderFrontiersAsPoints(world, {0, 0, 0, 220}, world.countryFrontiers, 1);
    highligthProvinceFrontiers(world, {255, 255, 0, 240},
                               world.selectedProvince);
    renderArmies(world, world.destRect);
    showSelectedArmiesPaths(world, world.destRect);
  }
}

inline void renderMap(World &world, bool isSecondMap) {

  // the second map is an offset map used to create the ilusion of a round world
  if (isSecondMap) {
    world.destRect.x = world.offsetX - world.texWidth * world.finalScale;
  }

  // base map
  displayTexture(world, world.height, 255);
  displayTexture(world, world.terrain, 200);

  renderMapModeLayer(world);
  
  renderFrontiers(world);
}