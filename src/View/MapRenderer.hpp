#pragma once

//=============================

#include "../Model/World.hpp"
#include "../utils.hpp"
#include "ArmyRenderer.hpp"
#include "FrontierRenderer.hpp"
#include "FrontierSmoothRenderer.hpp"
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

inline void renderArmiesLayer(World& world){
    renderArmies(world, world.destRect);
    showSelectedArmiesPaths(world, world.destRect);
}

inline void renderMap(World &world, bool isSecondMap) {

  // the second map is an offset map used to create the ilusion of a round world
  if (isSecondMap) {
    world.destRect.x = world.offsetX - world.texWidth * world.finalScale;
  }

  // base map  
  displayTexture(world, world.height, 255);
  displayTexture(world, world.terrain, 255);


  renderMapModeLayer(world);
  if(world.FRONTIER_MODE_SMOOTH){
    renderSmoothFrontiers(world);
  }else{
    renderFrontiers(world);
  }
  renderArmiesLayer(world);
}