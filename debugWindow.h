#pragma once
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "GameData.hpp"
#include "utils.hpp"

struct DebugWindow {
  SDL_Window*   window   = nullptr;
  SDL_Renderer* renderer = nullptr;
  bool visible = true;

  void init() {
    window = SDL_CreateWindow(
      "Debug",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      420, 620,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
  }

  void hide() {
    visible = false;
    SDL_HideWindow(window);
  }

  void processEvent(const SDL_Event& e) {
    if (!visible) return;
    ImGui_ImplSDL2_ProcessEvent(&e);
  }

  void render(const GameData& data) {
    if (!visible) return;

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)w, (float)h));
    ImGui::Begin("GameData Debug", nullptr,
      ImGuiWindowFlags_NoTitleBar |
      ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove
    );

    // ── General ──────────────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("General")) {
      ImGui::Text("FPS:        %.3f", data.fps);
      ImGui::Text("Scale:      %.3f", data.scale);
      ImGui::Text("Offset:     (%.1f, %.1f)", data.offsetX, data.offsetY);
      ImGui::Text("Dragging:   %s", data.dragging ? "yes" : "no");
      ImGui::Text("LastPos:    (%d, %d)", data.lastX, data.lastY);
      ImGui::Text("FrameDelay: %d", data.frameDelay);
      ImGui::Text("TexSize:    %d x %d", data.texWidth, data.texHeight);
    }

    // ── Load Results ─────────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Load Results")) {
      auto ok = [](bool v) { return v ? "OK" : "NULL"; };
      ImGui::Text("provincesBmp:           %s", ok(data.provincesBmp));
      ImGui::Text("terrain:                %s", ok(data.terrain));
      ImGui::Text("countries:              %s", ok(data.countries));
      ImGui::Separator();
      ImGui::Text("BmpColor->ProvinceId:   %zu entries", data.BmpColorToProvinceId.size());
      ImGui::Text("ProvinceId->CountryTag: %zu entries", data.ProvinceIdToCountryTag.size());
      ImGui::Text("CountryTag->Name:       %zu entries", data.CountryTagToCountryName.size());
      ImGui::Text("Name->Color:            %zu entries", data.CountryNameToCountryColor.size());
      ImGui::Text("Frontiers:              %zu entries", data.frontierList.size());
    }
    // ── SelectedProvince ─────────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Selected Province")) {
      uint32_t pc = data.selectedProvince;
      ImGui::Text("Province: %d (%s)", data.id, colorToString(pc).c_str());

      if (auto provinceId = mapFind(data.BmpColorToProvinceId, pc)) {
        if (auto tag = mapFind(data.ProvinceIdToCountryTag, *provinceId)) {
          uint32_t cc = getCountryColorFromProvinceColor(data, pc);
          ImGui::Text("Country: %s (%s)", tag->c_str(), colorToString(cc).c_str());
        }
      }
    }

    // ── Maps ─────────────────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Maps")) {
      constexpr int LIMIT = 10;

      // BmpColor -> ProvinceId
      if (ImGui::TreeNode("BmpColor -> ProvinceId")) {
        int n = 0;
        for (auto& [color, id] : data.BmpColorToProvinceId) {
          if (n++ >= LIMIT) { ImGui::TextDisabled("... (showing first %d)", LIMIT); break; }
          ImGui::Text("%s  ->  %u", colorToString(color).c_str(), id);
        }
        ImGui::TreePop();
      }

      // CountryTag -> CountryName
      if (ImGui::TreeNode("CountryTag -> CountryName")) {
        int n = 0;
        for (auto& [tag, name] : data.CountryTagToCountryName) {
          if (n++ >= LIMIT) { ImGui::TextDisabled("... (showing first %d)", LIMIT); break; }
          ImGui::Text("%s  ->  %s", tag.c_str(), name.c_str());
        }
        ImGui::TreePop();
      }

      // CountryName -> CountryColor
      if (ImGui::TreeNode("CountryName -> CountryColor")) {
        int n = 0;
        for (auto& [name, color] : data.CountryNameToCountryColor) {
          if (n++ >= LIMIT) { ImGui::TextDisabled("... (showing first %d)", LIMIT); break; }
          ImGui::Text("%s  ->  %s", name.c_str(), colorToString(color).c_str());
        }
        ImGui::TreePop();
      }

      // Frontiers (pair of province IDs as colors)
      if (ImGui::TreeNode("Frontiers")) {
        int n = 0;
        for (auto& [pair, pts] : data.frontierList) {
          if (n++ >= LIMIT) { ImGui::TextDisabled("... (showing first %d)", LIMIT); break; }
          ImGui::Text("%s <-> %s : %zu pts",
            colorToString(pair.first).c_str(),
            colorToString(pair.second).c_str(),
            pts.size());
        }
        ImGui::TreePop();
      }
    }

    ImGui::End();
    ImGui::Render();

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
  }

  void shutdown() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
  }
};