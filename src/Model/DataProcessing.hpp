#pragma once

//===========================

#include "World.hpp"
#include "../utils.hpp"
#include "../third_party/Polyline2D/Polyline2D.hpp"

//===========================

#include <cstdint>
#include <string>
#include <SDL2/SDL_image.h>
#include <unordered_set>
#include <SDL2/SDL_image.h>
#include <utility>   // std::pair, std::minmax
#include <algorithm> // std::minmax
#include <limits>    // std::numeric_limits
#include <cstdint>   // uint32_t

//============================

inline void generateFrontierStyle(
    World& world,
    const std::string& styleName,
    const std::map<std::pair<uint32_t,uint32_t>, std::vector<std::vector<SDL_FPoint>>>& worldFrontiers,
    float thickness,
    SDL_Color color
){
    FrontierStyle style;
    const float OFFSET = 0.50f;

    for (const auto& [key, segments] : worldFrontiers) {
        FrontierData data;
        bool hasAny = false;

        for (const auto& points : segments) {
            if (points.size() < 2) continue;

            if (!hasAny) {
                data.minX = data.maxX = points[0].x + OFFSET;
                data.minY = data.maxY = points[0].y + OFFSET;
                hasAny = true;
            }
            for (auto& p : points) {
                float ox = p.x + OFFSET;
                float oy = p.y + OFFSET;
                data.minX = std::min(data.minX, ox); data.maxX = std::max(data.maxX, ox);
                data.minY = std::min(data.minY, oy); data.maxY = std::max(data.maxY, oy);
            }

            std::vector<crushedpixel::Vec2> pts;
            pts.reserve(points.size());
            for (auto& p : points)
                pts.push_back(crushedpixel::Vec2{p.x + OFFSET, p.y + OFFSET});

            auto verts = crushedpixel::Polyline2D::create(
                pts, thickness,
                crushedpixel::Polyline2D::JointStyle::BEVEL,
                crushedpixel::Polyline2D::EndCapStyle::SQUARE
            );

            data.cachedVerts.reserve(data.cachedVerts.size() + verts.size());
            for (auto& v : verts) {
                SDL_Vertex sv;
                sv.position = SDL_FPoint{ v.x, v.y };
                sv.color = color;
                sv.tex_coord = SDL_FPoint{0,0};
                data.cachedVerts.push_back(sv);
            }
        }

        if (!hasAny) continue;

        style.frontiers[key] = std::move(data);
    }

    world.frontierCache[styleName] = std::move(style);
}

inline std::vector<std::vector<SDL_FPoint>> orderPoints(std::vector<SDL_FPoint> points) {
    std::vector<std::vector<SDL_FPoint>> segments;
    if (points.empty()) return segments;

    const float maxDist = 1.05f * 1.05f; // límite: recto=1, diagonal≈0.707

    while (!points.empty()) {
        std::vector<SDL_FPoint> ordered;

        // Semilla del nuevo segmento
        ordered.push_back(points[0]);
        points.erase(points.begin());

        auto extend = [&](bool front) {
            while (!points.empty()) {
                SDL_FPoint ref = front ? ordered.front() : ordered.back();
                int bestIdx = -1;
                float bestDist = std::numeric_limits<float>::max();

                for (int i = 0; i < (int)points.size(); i++) {
                    float dx = points[i].x - ref.x;
                    float dy = points[i].y - ref.y;
                    float dist = dx * dx + dy * dy;
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestIdx = i;
                    }
                }

                if (bestIdx == -1 || bestDist > maxDist) break;

                if (front) ordered.insert(ordered.begin(), points[bestIdx]);
                else ordered.push_back(points[bestIdx]);
                points.erase(points.begin() + bestIdx);
            }
        };

        extend(false); // hacia adelante
        extend(true);  // hacia atrás

        segments.push_back(std::move(ordered));
    }

    return segments;
}
inline std::vector<std::vector<SDL_FPoint>> randomizePositions(std::vector<SDL_FPoint> points) {
    std::vector<std::vector<SDL_FPoint>> segments;
    if (points.empty()) return segments;

    const float jitterRange = 0.25f; // randomization value

    auto jitter = [&](float range) {
        return ((float)rand() / RAND_MAX) * 2.0f * range - range;
    };

    for (size_t i = 0; i < points.size(); i++) {
        if (i == 0 || i == points.size() - 1) continue; // no mover extremos
        points[i].x += jitter(jitterRange);
        points[i].y += jitter(jitterRange);
    }

    segments.push_back(std::move(points));
    return segments;
}

inline void findFrontiers(World& world) {
    SDL_Surface* img = world.provincesBmp;
    std::map<std::pair<uint32_t, uint32_t>, std::vector<std::vector<SDL_FPoint>>> frontierList;
    int imgW = img->w;
    int imgH = img->h;

    // Acumulador temporal: puntos sueltos por frontera, antes de ordenar
    std::map<std::pair<uint32_t, uint32_t>, std::vector<SDL_FPoint>> rawPoints;

    for (int y = 0; y < imgH; y++) {
        for (int x = 0; x < imgW; x++) {
            uint32_t current = getPixelColor(img, x, y);
            if (x + 1 < imgW) {
                uint32_t next = getPixelColor(img, x + 1, y);
                if (next != current) {
                    auto key = std::minmax(current, next);
                    rawPoints[key].push_back({x + 0.5f, (float)y});
                }
            }
        }
    }

    for (int y = 0; y < imgH; y++) {
        for (int x = 0; x < imgW; x++) {
            uint32_t current = getPixelColor(img, x, y);
            if (y + 1 < imgH) {
                uint32_t next = getPixelColor(img, x, y + 1);
                if (next != current) {
                    auto key = std::minmax(current, next);
                    rawPoints[key].push_back({(float)x, y + 0.5f});
                }
            }
        }
    }

    for (auto& [key, pts] : rawPoints) {
        auto ordered = orderPoints(pts); // vector<vector<SDL_FPoint>>, ya segmentado
        for (auto& segment : ordered)
            segment = randomizePositions(segment)[0]; // jitter por segmento
        frontierList[key] = std::move(ordered);
    }

    world.provinceFrontiers = frontierList;
}
// ===============================================================================================================
// Adjacency Graph
// ===============================================================================================================

inline void buildAdjacency(World& world) {
    const std::map<std::pair<uint32_t, uint32_t>, std::vector<std::vector<SDL_FPoint>>>& provinceFrontiers = world.provinceFrontiers;
    const std::list<Province>& provinces = world.provinces;
    std::map<int, std::vector<int>> adjacency;
    for (const auto& [pair, _] : provinceFrontiers) {
        Province* a = provinceFindByColor(provinces, pair.first);
        Province* b = provinceFindByColor(provinces, pair.second);
        if (!a || !b) continue;
        adjacency[a->id].push_back(b->id);
        adjacency[b->id].push_back(a->id);
    }
    world.adjacencyGraph = adjacency;
}

inline std::map<int, std::vector<int>> buildAccessibilityGraph(const World& world, const std::vector<std::string>& accessibleCountryTags) {

    const std::unordered_set<std::string> accessible(
        accessibleCountryTags.begin(), accessibleCountryTags.end());

    // Find the highest province id, to size a flat lookup vector (avoids map lookups per id)
    int maxId = 0;
    for (auto& p : world.provinces) maxId = std::max(maxId, p.id);

    // Flat array: id -> pointer to Province (nullptr if id doesn't exist)
    std::vector<const Province*> idMap(maxId + 1, nullptr);
    for (auto& p : world.provinces) idMap[p.id] = &p;

    // Flat array: id -> is this province "accessible" (owner is in the accessible set)
    // AND not a mountain. Mountains are always excluded, regardless of owner.
    std::vector<char> isAccessible(maxId + 1, 0);
    for (int id = 0; id <= maxId; ++id) {
        const Province* p = idMap[id];
        if (!p) continue;

        // Skip mountains entirely: they never count as accessible / never appear in the graph
        if (p->terrainType == TerrainType::MOUNTAIN) continue;

        if (p->terrainType == TerrainType::OCEAN) continue;

        if (accessible.count(p->owner))
            isAccessible[id] = 1;
    }

    std::map<int, std::vector<int>> adjacency;

    // Build adjacency only between provinces that are accessible (
    for (const auto& [provinceId, neighbors] : world.adjacencyGraph) {
        if (provinceId > maxId || !isAccessible[provinceId]) continue;

        std::vector<int> out;
        out.reserve(neighbors.size());
        for (int neighborId : neighbors)
            if (neighborId <= maxId && isAccessible[neighborId])
                out.push_back(neighborId);

        if (!out.empty())
            adjacency.emplace(provinceId, std::move(out));
    }

    return adjacency;
}


inline void InitAllAccesibiltyGraphs(World& world){
    for (auto& country : world.countries) {
        country.accessibilityGraph = buildAccessibilityGraph(world, country.accessibleCountries);
    }
}

inline void reloadAccesibilityGraph(World& world, Country* country){
        country -> accessibilityGraph = buildAccessibilityGraph(world, country -> accessibleCountries);
}

// ===============================================================================================================
// Frontiers
// ===============================================================================================================

inline void findFrontiersBetweenCountries(World& world) {
    std::map<std::pair<uint32_t, uint32_t>, std::vector<std::vector<SDL_FPoint>>>& frontiers = world.provinceFrontiers;
    std::map<std::pair<uint32_t, uint32_t>, std::vector<std::vector<SDL_FPoint>>> filteredFrontiers;

    for (const auto& [key, points] : frontiers) {
        const Province* province1 = provinceFindByColor(world.provinces, key.first);
        const Province* province2 = provinceFindByColor(world.provinces, key.second);

        if (province1 == nullptr || province2 == nullptr) continue;

        const bool differentCountries = province1->owner != province2->owner
            && !province1->owner.empty()
            && !province2->owner.empty();

        const bool oneIsCountryOneIsNot = province1->owner.empty() != province2->owner.empty();

        if (differentCountries || oneIsCountryOneIsNot) {
            filteredFrontiers[key] = points;
        }
    }

    world.countryFrontiers = filteredFrontiers;
}

// ===============================================================================================================
// Provinces
// ===============================================================================================================

inline std::map<uint32_t, SDL_Point> initProvincesCenters(const World& world) {
    struct Accum {
        int64_t sumX = 0;
        int64_t sumY = 0;
        int count = 0;
    };

    std::unordered_map<uint32_t, Accum> accum;
    accum.reserve(1024);

    for (int y = 0; y < world.texHeight; y++) {
        for (int x = 0; x < world.texWidth; x++) {
            uint32_t color = getPixelColor(world.provincesBmp, x, y);
            auto& a = accum[color];
            a.sumX += x;
            a.sumY += y;
            a.count++;
        }
    }

    std::map<uint32_t, SDL_Point> centerList;
    for (auto& [color, a] : accum)
        centerList[color] = { (int)(a.sumX / a.count), (int)(a.sumY / a.count) };

    return centerList;
}

inline void buildCountriesLayer(World& world) {
    SDL_Renderer* renderer = world.renderer;

    SDL_Surface* provinces = world.provincesBmp;
    if (!provinces) return;

    SDL_Surface* result = SDL_CreateRGBSurfaceWithFormat(
        0, provinces->w, provinces->h, 32, SDL_PIXELFORMAT_RGBA32
    );
    if (!result) return;

    SDL_FillRect(result, nullptr, SDL_MapRGBA(result->format, 0, 0, 0, 0));

    std::map<uint32_t, uint32_t> colorToCountryColor;

    SDL_LockSurface(provinces);
    SDL_LockSurface(result);

    int imgW = provinces->w;
    int imgH = provinces->h;

    // Precompute black in the result surface's pixel format, used for mountain terrain

    uint32_t mountainColor = SDL_MapRGB(result->format, 55, 55, 55);

    for (int y = 0; y < imgH; y++) {
        for (int x = 0; x < imgW; x++) {
            uint32_t pixelColor = getPixelColor(provinces, x, y);

            auto it = colorToCountryColor.find(pixelColor);
            if (it != colorToCountryColor.end()) {
                if (it->second != 0) setPixel(result, x, y, it->second);
                continue;
            }

            Province* p = provinceFindByColor(world.provinces, pixelColor);
            if (!p) {
                colorToCountryColor[pixelColor] = 0;
                continue;
            }

            // If the province terrain is MOUNTAIN, it is painted in a color.

            if (p->terrainType == TerrainType::MOUNTAIN) {
                colorToCountryColor[pixelColor] = mountainColor;
                setPixel(result, x, y, mountainColor);
                continue;
            }
            
            // If the province has no country in it, don't paint it.

            if (p->owner.empty()) {
                colorToCountryColor[pixelColor] = 0;
                continue;
            }

            // Find the country, if the country doesnt exist on the list, don't paint it.

            Country* c = findCountryByTag(world.countries, p->owner);
            if (!c) {
                colorToCountryColor[pixelColor] = 0;
                continue;
            }

            // Find the country color

            uint32_t countryColor = SDL_MapRGB(result->format, c->color.r, c->color.g, c->color.b);

            colorToCountryColor[pixelColor] = countryColor;

            // paint it
            
            setPixel(result, x, y, countryColor);
        }
    }

    SDL_UnlockSurface(provinces);
    SDL_UnlockSurface(result);

    world.countriesImg = result;
}