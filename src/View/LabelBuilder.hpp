#pragma once

#include "../Model/World.hpp"
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <algorithm>

// ============================================================
// Estructuras
// ============================================================

// Umbral mínimo de píxeles para que una región sea considerada
// (evita poner nombres en ruido de 2-3 píxeles sueltos)
inline constexpr int MIN_REGION_AREA = 40;

// ============================================================
// Helpers de píxeles
// ============================================================

inline SDL_Color getPixelColorLabel(SDL_Surface* surf, int x, int y) {
    Uint32* pixels = static_cast<Uint32*>(surf->pixels);
    Uint32 pixel = pixels[y * (surf->pitch / 4) + x];
    SDL_Color c;
    SDL_GetRGBA(pixel, surf->format, &c.r, &c.g, &c.b, &c.a);
    return c;
}

inline bool sameColor(const SDL_Color& a, const SDL_Color& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

// ============================================================
// Distance transform (chamfer 3-4) sobre una máscara local
// Devuelve el punto (coords locales) más alejado del borde de la región
// ============================================================

inline SDL_Point chamferPole(const std::vector<bool>& mask, int w, int h) {
    const int INF = 1 << 28;
    std::vector<int> dist(static_cast<size_t>(w) * h, INF);

    for (size_t i = 0; i < mask.size(); ++i) {
        if (!mask[i]) dist[i] = 0;
    }

    auto at = [&](int x, int y) -> int& { return dist[static_cast<size_t>(y) * w + x]; };

    // pasada hacia adelante
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (!mask[static_cast<size_t>(y) * w + x]) continue;
            int best = at(x, y);
            if (x > 0)            best = std::min(best, at(x - 1, y) + 3);
            if (y > 0)            best = std::min(best, at(x, y - 1) + 3);
            if (x > 0 && y > 0)   best = std::min(best, at(x - 1, y - 1) + 4);
            if (x < w - 1 && y > 0) best = std::min(best, at(x + 1, y - 1) + 4);
            at(x, y) = best;
        }
    }

    // pasada hacia atrás
    for (int y = h - 1; y >= 0; --y) {
        for (int x = w - 1; x >= 0; --x) {
            if (!mask[static_cast<size_t>(y) * w + x]) continue;
            int best = at(x, y);
            if (x < w - 1)            best = std::min(best, at(x + 1, y) + 3);
            if (y < h - 1)            best = std::min(best, at(x, y + 1) + 3);
            if (x < w - 1 && y < h - 1) best = std::min(best, at(x + 1, y + 1) + 4);
            if (x > 0 && y < h - 1)   best = std::min(best, at(x - 1, y + 1) + 4);
            at(x, y) = best;
        }
    }

    int bestDist = -1;
    SDL_Point bestPoint{ 0, 0 };
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            if (!mask[static_cast<size_t>(y) * w + x]) continue;
            int d = at(x, y);
            if (d > bestDist) {
                bestDist = d;
                bestPoint = { x, y };
            }
        }
    }
    return bestPoint;
}

// ============================================================
// Flood fill para separar countriesImg en regiones conexas,
// y cálculo del pole of inaccessibility de cada una
// ============================================================

inline std::vector<Region> findRegionsWithPoles(SDL_Surface* surf) {
    int w = surf->w, h = surf->h;
    std::vector<int> labelMap(static_cast<size_t>(w) * h, -1);
    std::vector<Region> regions;

    SDL_LockSurface(surf);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            if (labelMap[idx] != -1) continue;

            SDL_Color c = getPixelColorLabel(surf, x, y);
            if (c.a == 0) continue; // transparente = sin dueño (mar, etc.)

            int regionId = static_cast<int>(regions.size());
            int minX = x, minY = y, maxX = x, maxY = y, area = 0;

            std::queue<SDL_Point> q;
            q.push({ x, y });
            labelMap[idx] = regionId;

            while (!q.empty()) {
                SDL_Point p = q.front(); q.pop();
                area++;
                minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
                minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);

                static const int dx[4] = { 1, -1, 0, 0 };
                static const int dy[4] = { 0, 0, 1, -1 };
                for (int k = 0; k < 4; ++k) {
                    int nx = p.x + dx[k], ny = p.y + dy[k];
                    if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
                    int nidx = ny * w + nx;
                    if (labelMap[nidx] != -1) continue;

                    SDL_Color nc = getPixelColorLabel(surf, nx, ny);
                    if (!sameColor(nc, c)) continue;

                    labelMap[nidx] = regionId;
                    q.push({ nx, ny });
                }
            }

            Region region;
            region.color = c;
            region.area = area;
            region.bbox = { minX, minY, maxX - minX + 1, maxY - minY + 1 };
            regions.push_back(region);
        }
    }

    SDL_UnlockSurface(surf);

    // distance transform por región, acotado a su bounding box (+1px padding)
    for (size_t i = 0; i < regions.size(); ++i) {
        Region& r = regions[i];
        int pw = r.bbox.w + 2;
        int ph = r.bbox.h + 2;

        std::vector<bool> mask(static_cast<size_t>(pw) * ph, false);
        for (int y = 0; y < r.bbox.h; ++y) {
            for (int x = 0; x < r.bbox.w; ++x) {
                int gx = r.bbox.x + x, gy = r.bbox.y + y;
                if (labelMap[static_cast<size_t>(gy) * w + gx] == static_cast<int>(i)) {
                    mask[static_cast<size_t>(y + 1) * pw + (x + 1)] = true;
                }
            }
        }

        SDL_Point localPole = chamferPole(mask, pw, ph);
        r.pole = {
            r.bbox.x + localPole.x - 1,
            r.bbox.y + localPole.y - 1
        };
    }

    return regions;
}

// ============================================================
// Busca en world.countries el país cuyo color coincide con el de la región.
// Devuelve {tag, name}; si no matchea ninguno, {"", ""}.
// ============================================================

inline std::pair<std::string, std::string> resolveCountry(World& world, const SDL_Color& color) {
    for (const auto& country : world.countries) {
        if (sameColor(country.color, color)) {
            return { country.tag, country.name };
        }
    }
    return { "", "" };
}

// ============================================================
// Punto de entrada: recalcula todas las labels de país.
// Llamar una sola vez al cargar el mapa, o cuando cambian fronteras
// (conquista, cesión de territorio, etc.) — nunca por frame.
// ============================================================

inline void buildCountryLabels(World& world) {
    world.countryLabels.clear();

    if (!world.countriesImg) return;

    std::vector<Region> regions = findRegionsWithPoles(world.countriesImg);

    for (const Region& r : regions) {
        if (r.area < MIN_REGION_AREA) continue;

        auto [tag, name] = resolveCountry(world, r.color);
        if (tag.empty()) continue; // color sin país asociado (mar, sin dueño, etc.)

        // Nos quedamos con la región más grande de cada país
        auto it = world.countryLabels.find(tag);
        if (it == world.countryLabels.end() || r.area > it->second.area) {
            world.countryLabels[tag] = CountryLabel{ tag, name, r.pole, r.area };
        }
    }
}