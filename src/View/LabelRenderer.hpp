#pragma once

#include "../Model/World.hpp"
#include "../Model/FontLoader.hpp"
#include <string>
#include <iostream>

// ============================================================
// Elige la fuente según el área (en píxeles) de la región del país.
// Ajustá estos umbrales a ojo probando con tu mapa real.
// Requiere que existan fuentes "country_small" / "country_medium" /
// "country_large" cargadas en initFonts (ver nota abajo).
// ============================================================

inline std::string pickFontForArea(int area) {
    if (area < 1500)  return "country_small";
    if (area < 8000)  return "country_medium";
    return "country_large";
}

// ============================================================
// Dibuja un punto/cruz bien visible en 'pos', sin depender de fuentes.
// Sirve para confirmar que la posición calculada cae donde esperamos,
// aunque el texto todavía no se vea por algún problema de fuente/cache.
// ============================================================

inline void renderDebugMarker(SDL_Renderer* renderer, SDL_Point pos, int size = 4) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // rojo bien visible

    SDL_Rect dot = { pos.x - size, pos.y - size, size * 2, size * 2 };
    SDL_RenderFillRect(renderer, &dot);

    // cruz extra para que se note incluso si el punto es muy chico
    SDL_RenderDrawLine(renderer, pos.x - size * 2, pos.y, pos.x + size * 2, pos.y);
    SDL_RenderDrawLine(renderer, pos.x, pos.y - size * 2, pos.x, pos.y + size * 2);
}

// ============================================================
// Dibuja todas las labels de país centradas en su posición.
// Si tu mapa tiene cámara/zoom, transformá 'label.position' de
// coordenadas de mundo a coordenadas de pantalla ANTES de armar
// el SDL_Rect (marcado abajo con TODO).
// ============================================================

inline void renderCountryLabels(World& world, SDL_FRect destRect) {
    std::cout << "[LabelRenderer] countryLabels a dibujar: " << world.countryLabels.size() << "\n";

    if (world.countryLabels.empty()) {
        std::cout << "[LabelRenderer] AVISO: world.countryLabels esta vacio. "
                     "Te olvidaste de llamar buildCountryLabels(world)?\n";
        return;
    }

    for (const auto& [tag, label] : world.countryLabels) {
        // misma transformación mundo -> pantalla que usa renderArmies
        float sx = destRect.x + label.position.x * world.finalScale;
        float sy = destRect.y + label.position.y * world.finalScale;

        if (sx < 0 || sy < 0 || sx > world.winWidth || sy > world.winHeight) continue;

        std::cout << "[LabelRenderer] tag=" << tag
                   << " name=" << label.countryName
                   << " world=(" << label.position.x << ", " << label.position.y << ")"
                   << " screen=(" << sx << ", " << sy << ")"
                   << " area=" << label.area << "\n";

        SDL_Point screenPos{ (int)sx, (int)sy };

        // --- Paso 1: dibujar el punto siempre, para validar la posicion ---
        renderDebugMarker(world.renderer, screenPos);

        // --- Paso 2: intentar el texto real ---
        std::string fontId = pickFontForArea(label.area);
        std::cout << "[LabelRenderer]   fontId elegido: " << fontId << "\n";

        TextCache& cache = getOrRenderText(world, fontId, label.countryName);

        if (!cache.texture) {
            std::cout << "[LabelRenderer]   ERROR: no se pudo generar textura de texto "
                         "(revisar que la fuente '" << fontId << "' este cargada en initFonts)\n";
            continue;
        }

        std::cout << "[LabelRenderer]   texto ok, tamaño=(" << cache.w << ", " << cache.h << ")\n";

        SDL_Rect dst;
        dst.w = cache.w;
        dst.h = cache.h;
        dst.x = screenPos.x - cache.w / 2;
        dst.y = screenPos.y - cache.h / 2;

        SDL_RenderCopy(world.renderer, cache.texture, nullptr, &dst);
    }
}