#pragma once

#include "../Model/World.hpp"
#include "../Model/FontLoader.hpp"
#include "../utils.hpp"
#include <algorithm>
#include <string>

// Pick the font size based on the country region area.
inline std::string pickFontForArea(int area) {
    if (area < 1500) return "country_small";
    if (area < 8000) return "country_medium";
    return "country_large";
}

inline float interpolate(float start, float end, float t) {
    return start + (end - start) * t;
}

// Draw country labels using the flag as the anchor and the name above it.
inline void renderCountryLabels(World& world, SDL_FRect destRect) {
    // Lower bound for the text to avoid showing names that are too wide for the country.
    constexpr float MAX_NAME_WIDTH_RATIO = 0.28f;

    for (const auto& [tag, label] : world.countryLabels) {
        // Interpolate the reveal threshold from the biggest countries to the smallest ones.
        // Bigger countries appear earlier; smaller countries wait until the end of the range.
        const float sizeRatio = std::clamp(static_cast<float>(label.area) / 20000.0f, 0.0f, 1.0f);
        const float flagZoomMin = interpolate(world.COUNTRY_FLAG_LAST_VISIBLE,
                                             world.COUNTRY_FLAG_FIRST_VISIBLE,
                                             sizeRatio);
        const float nameZoomMin = interpolate(world.COUNTRY_NAME_LAST_VISIBLE,
                                             world.COUNTRY_NAME_FIRST_VISIBLE,
                                             sizeRatio);

        float sx = destRect.x + label.position.x * world.finalScale;
        float sy = destRect.y + label.position.y * world.finalScale;

        if (sx < 0 || sy < 0 || sx > world.winWidth || sy > world.winHeight) continue;

        SDL_Point screenPos{ (int)sx, (int)sy };

        Country* country = findCountryByTag(world.countries, tag);
        SDL_Texture* flagTexture = country ? country->flag : nullptr;

        std::string fontId = pickFontForArea(label.area);
        TextCache& cache = getOrRenderText(world, fontId, label.countryName);

        const float nameFits = (cache.texture && label.area > 0 && cache.w <= label.area * MAX_NAME_WIDTH_RATIO) ? 1.0f : 0.0f;

        const bool showFlag = world.finalScale >= flagZoomMin;
        const bool showName = showFlag && nameFits > 0.0f && world.finalScale >= nameZoomMin;

        if (!showFlag && !showName) {
            continue;
        }

        const int textHeight = std::max(1, cache.h);
        const float zoomBoost = 1.0f + std::max(0.0f, world.finalScale - 1.0f) * 0.08f;
        const float flagScale = 1.5f;
        const int displayFlagSize = flagTexture ? static_cast<int>(std::max(6, static_cast<int>(textHeight * 0.62f)) * flagScale * zoomBoost) : 0;

        if (flagTexture && showFlag) {
            SDL_Rect flagRect;
            flagRect.w = displayFlagSize;
            flagRect.h = displayFlagSize;
            flagRect.x = screenPos.x - displayFlagSize / 2;
            flagRect.y = screenPos.y + static_cast<int>(displayFlagSize * 0.08f) - displayFlagSize / 2;

            SDL_SetRenderDrawColor(world.renderer, 0, 0, 0, 255);
            SDL_Rect borderRect = flagRect;
            borderRect.x -= 1;
            borderRect.y -= 1;
            borderRect.w += 2;
            borderRect.h += 2;
            SDL_RenderDrawRect(world.renderer, &borderRect);
            SDL_RenderDrawRect(world.renderer, &flagRect);

            SDL_RenderCopy(world.renderer, flagTexture, nullptr, &flagRect);
        }

        if (cache.texture && showName) {
            SDL_Rect textRect;
            textRect.w = cache.w;
            textRect.h = cache.h;
            textRect.x = screenPos.x - cache.w / 2;
            textRect.y = screenPos.y - cache.h - 8 - displayFlagSize / 2 - static_cast<int>(displayFlagSize * 0.12f);
            SDL_RenderCopy(world.renderer, cache.texture, nullptr, &textRect);
        }
    }
}