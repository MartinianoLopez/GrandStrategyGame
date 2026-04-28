#include "GameData.hpp"
#include "utils.hpp"

GameData::GameData()
    : provincesBmp(nullptr),
      texWidth(0),
      texHeight(0),
      scale(1.f),
      offsetX(0.f),
      offsetY(0.f),
      dragging(false),
      lastX(0),
      lastY(0),
      frameDelay(1000 / 60),
      selectedProvince(0),
      provinceId(""),
      id(0)
{}