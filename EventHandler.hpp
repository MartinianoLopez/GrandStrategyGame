#pragma once
#include <SDL2/SDL.h>
#include <algorithm>
#include "GameData.hpp"
#include "utils.hpp"

static void processEvent(GameData& state, SDL_Window* window, bool& running, const SDL_Event& event) {
  switch (event.type) {
    case SDL_QUIT:
      running = false;
      break;
    case SDL_MOUSEWHEEL: {
      float zoomFactor = (event.wheel.y > 0) ? 1.1f : 0.9f;
      float mx = (float)event.wheel.mouseX;
      float my = (float)event.wheel.mouseY;
      state.offsetX = mx + (state.offsetX - mx) * zoomFactor;
      state.offsetY = my + (state.offsetY - my) * zoomFactor;
      state.scale *= zoomFactor;
      break;
    }
    case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_RIGHT) {
          state.dragging = true;
          state.lastX = event.button.x;
          state.lastY = event.button.y;

          int winWidth, winHeight;
          SDL_GetWindowSize(window, &winWidth, &winHeight);
          float baseScale = std::min((float)winWidth / state.texWidth, (float)winHeight / state.texHeight);
          float finalScale = baseScale * state.scale;
          int imgX = static_cast<int>((event.button.x - state.offsetX) / finalScale);
          int imgY = static_cast<int>((event.button.y - state.offsetY) / finalScale);
          if (imgX >= 0 && imgX < state.texWidth && imgY >= 0 && imgY < state.texHeight) {
              uint32_t targetColor = getPixelColor(state.provincesBmp, imgX, imgY);
              auto it = state.BmpColorToProvinceId.find(targetColor);
              if (it != state.BmpColorToProvinceId.end()) {
                  state.secundarySelectedProvinceId = it->second;
                  if (state.selectedProvinceId != 0 && searchTroops(state, state.selectedProvinceId) != 0)
                moveTroops(state, state.selectedProvinceId, state.secundarySelectedProvinceId);
            }
        }
      }
      if (event.button.button == SDL_BUTTON_LEFT) {
        int winWidth, winHeight;
        SDL_GetWindowSize(window, &winWidth, &winHeight);
        float baseScale = std::min(
          (float)winWidth / state.texWidth,
          (float)winHeight / state.texHeight
        );
        float finalScale = baseScale * state.scale;
        float mx = (float)event.button.x;
        float my = (float)event.button.y;
        int imgX = static_cast<int>((mx - state.offsetX) / finalScale);
        int imgY = static_cast<int>((my - state.offsetY) / finalScale);
        if (imgX >= 0 && imgX < state.texWidth &&
            imgY >= 0 && imgY < state.texHeight) {
          uint32_t provinceColor = getPixelColor(state.provincesBmp, imgX, imgY);
          auto it = state.BmpColorToProvinceId.find(provinceColor);
          if (it == state.BmpColorToProvinceId.end()) {
            std::cerr << "Color not found: " << colorToString(provinceColor) << "\n";
            break;
          }
          state.selectedProvince = provinceColor;
          uint32_t provinceId = it->second;
          state.selectedProvinceId = provinceId;
          state.id = provinceId;
          auto itOwner = state.ProvinceIdToCountryTag.find(provinceId);
          std::string owner = (itOwner != state.ProvinceIdToCountryTag.end())
            ? itOwner->second : "UNKNOWN";
          /* 
          std::cerr << "------------------------\n";
          std::cerr << "Province: " << provinceId << " (" << colorToString(provinceColor) << ")\n";
          uint32_t countryColor = getCountryColorFromProvinceColor(state, provinceColor);
          std::cerr << "Country: " << owner << " (" << colorToString(countryColor) << ")\n";
          */
        }
      }
      break;
    
    case SDL_MOUSEBUTTONUP:
      if (event.button.button == SDL_BUTTON_RIGHT)
        state.dragging = false;
      break;
    case SDL_MOUSEMOTION:
      if (state.dragging) {
        state.offsetX += event.motion.x - state.lastX;
        state.offsetY += event.motion.y - state.lastY;
        state.lastX = event.motion.x;
        state.lastY = event.motion.y;
      }
      break;
  }
}

void handleEvents(GameData& state, SDL_Window* window, bool& running) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    processEvent(state, window, running, event);
  }
}

void handleEvent(GameData& state, SDL_Window* window, bool& running, const SDL_Event& event) {
  processEvent(state, window, running, event);
}