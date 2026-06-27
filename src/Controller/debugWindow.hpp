#pragma once

#include "imgui.h" 
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "../Model/World.hpp"
#include "../utils.hpp"

#ifdef name
#undef name
#endif

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

    void render(const World& world) {
        if (!visible) return;

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)w, (float)h));
        ImGui::Begin("##debug", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoMove
        );

        // ── General ──────────────────────────────────────────────────────────
        if (ImGui::CollapsingHeader("General")) {
            ImGui::Text("FPS         %.1f", world.fps);
            ImGui::Text("Scale       %.3f  (final %.3f)", world.scale, world.finalScale);
            ImGui::Text("Offset      %.1f, %.1f", world.offsetX, world.offsetY);
            ImGui::Text("TexSize     %d x %d", world.texWidth, world.texHeight);
            ImGui::Text("FrameDelay  %d ms", world.frameDelay);
            ImGui::Text("Dragging    %s", world.dragging ? "yes" : "no");
            ImGui::Text("Player      %s", world.playerCountry.c_str());
        }

        // ── Assets ───────────────────────────────────────────────────────────
        if (ImGui::CollapsingHeader("Assets")) {
            auto ok = [](bool v){ return v ? "OK" : "null"; };
            ImGui::Text("provincesBmp  %s", ok(world.provincesBmp));
            ImGui::Text("terrain       %s", ok(world.terrain));
            ImGui::Text("height        %s", ok(world.height));
        }

        // ── Selected Province ─────────────────────────────────────────────────
        if (ImGui::CollapsingHeader("Selected Province")) {
            const Province* p = provinceFindById(world.provinces, world.selectedProvince);
            if (p) {
                ImGui::Text("ID      %d", p->id);
                ImGui::Text("Name    %s", p->name.c_str());
                ImGui::Text("Owner   %s", p->owner.empty() ? "none" : p->owner.c_str());
                ImGui::Text("Controller   %s", p->controller.empty() ? "none" : p->controller.c_str());
                ImGui::Text("Color   %d %d %d", p->color.r, p->color.g, p->color.b);
                ImGui::Text("Center  %d, %d", p->center.x, p->center.y);
            } else {
                ImGui::TextDisabled("none");
            }
        }

        // ── Secondary Selected Province ───────────────────────────────────────
        if (ImGui::CollapsingHeader("Secondary Province")) {
            const Province* p = provinceFindById(world.provinces, world.objectiveProvince);
            if (p) {
                ImGui::Text("ID      %d", p->id);
                ImGui::Text("Name    %s", p->name.c_str());
                ImGui::Text("Owner   %s", p->owner.empty() ? "none" : p->owner.c_str());
                ImGui::Text("Color   %d %d %d", p->color.r, p->color.g, p->color.b);
                ImGui::Text("Center  %d, %d", p->center.x, p->center.y);
            } else {
                ImGui::TextDisabled("none");
            }
        }

        // ── Provinces ─────────────────────────────────────────────────────────
        if (ImGui::CollapsingHeader("Provinces")) {
            ImGui::Text("Total: %zu", world.provinces.size());
            ImGui::Separator();
            constexpr int LIMIT = 12;
            int n = 0;
            for (const auto& p : world.provinces) {
                if (n++ >= LIMIT) { ImGui::TextDisabled("... %zu total", world.provinces.size()); break; }
                ImGui::Text("[%d] %-20s  owner: %-4s  rgb(%d,%d,%d)",
                    p.id,
                    p.name.c_str(),
                    p.owner.empty() ? "-" : p.owner.c_str(),
                    p.color.r, p.color.g, p.color.b
                );
            }
        }

        // ── Countries ─────────────────────────────────────────────────────────
        if (ImGui::CollapsingHeader("Countries")) {
            ImGui::Text("Total: %zu", world.countries.size());
            ImGui::Separator();
            for (const auto& c : world.countries) {
                ImGui::Text("%-4s  %-20s  money: %d  rgb(%d,%d,%d)",
                    c.tag.c_str(),
                    c.name.c_str(),
                    c.money,
                    c.color.r, c.color.g, c.color.b
                );
            }
        }

        // ── Armies ────────────────────────────────────────────────────────────
        if (ImGui::CollapsingHeader("Armies")) {
            ImGui::Text("Total: %zu", world.armies.size());
            ImGui::Separator();
            for (const auto& a : world.armies) {
                ImGui::Text("%-20s  owner: %-4s  pos: %d  power: %d",
                    a.name.c_str(),
                    a.owner.c_str(),
                    a.position,
                    a.power
                );
            }
        }

        // ── Frontiers ─────────────────────────────────────────────────────────
        if (ImGui::CollapsingHeader("Frontiers")) {
            ImGui::Text("Province frontiers:  %zu", world.provinceFrontiers.size());
            ImGui::Text("Country frontiers:   %zu", world.countryFrontiers.size());
        }

        ImGui::End();
        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
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