#include <vector>
#include <queue>
#include <cmath>
#include <limits>
#include <algorithm>

struct Point { double x, y; };

// Distancia con signo de un punto al polígono (positiva adentro, negativa afuera)
inline double pointToSegmentDist(const Point& p, const Point& a, const Point& b) {
    double dx = b.x - a.x, dy = b.y - a.y;
    if (dx == 0 && dy == 0) {
        dx = p.x - a.x; dy = p.y - a.y;
        return std::sqrt(dx*dx + dy*dy);
    }
    double t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / (dx*dx + dy*dy);
    t = std::max(0.0, std::min(1.0, t));
    double projX = a.x + t * dx, projY = a.y + t * dy;
    double ddx = p.x - projX, ddy = p.y - projY;
    return std::sqrt(ddx*ddx + ddy*ddy);
}

inline bool pointInPolygon(const Point& p, const std::vector<Point>& poly) {
    bool inside = false;
    size_t n = poly.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const Point& pi = poly[i];
        const Point& pj = poly[j];
        if (((pi.y > p.y) != (pj.y > p.y)) &&
            (p.x < (pj.x - pi.x) * (p.y - pi.y) / (pj.y - pi.y) + pi.x)) {
            inside = !inside;
        }
    }
    return inside;
}

inline double signedDistance(const Point& p, const std::vector<Point>& poly) {
    double minDist = std::numeric_limits<double>::max();
    size_t n = poly.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        minDist = std::min(minDist, pointToSegmentDist(p, poly[j], poly[i]));
    }
    return pointInPolygon(p, poly) ? minDist : -minDist;
}

struct Cell {
    Point c;       // centro de la celda
    double h;      // mitad del lado de la celda
    double d;      // distancia con signo al borde (en c)
    double max;    // cota superior optimista de distancia dentro de esta celda

    Cell(Point center, double half, const std::vector<Point>& poly)
        : c(center), h(half) {
        d = signedDistance(c, poly);
        max = d + h * std::sqrt(2.0); // cota superior: distancia + diagonal media
    }
};

struct CompareCell {
    bool operator()(const Cell& a, const Cell& b) const {
        return a.max < b.max; // priority_queue es max-heap por defecto
    }
};

inline Point polylabel(const std::vector<Point>& poly, double precision = 1.0) {
    // Bounding box
    double minX = poly[0].x, minY = poly[0].y, maxX = poly[0].x, maxY = poly[0].y;
    for (const auto& p : poly) {
        minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
    }
    double width = maxX - minX, height = maxY - minY;
    double cellSize = std::min(width, height);
    double h = cellSize / 2.0;

    if (cellSize == 0) return { minX, minY };

    std::priority_queue<Cell, std::vector<Cell>, CompareCell> queue;

    // Sembrar la grilla inicial
    for (double x = minX; x < maxX; x += cellSize) {
        for (double y = minY; y < maxY; y += cellSize) {
            queue.push(Cell({ x + h, y + h }, h, poly));
        }
    }

    // Candidato inicial: el centroide (suele acelerar la convergencia)
    // (opcional, podés usar tu función polygonCentroid de antes)
    Cell best = queue.top();

    while (!queue.empty()) {
        Cell current = queue.top();
        queue.pop();

        if (current.d > best.d) {
            best = current;
        }

        // Si esta celda no puede superar al mejor encontrado, descartarla
        if (current.max - best.d <= precision) continue;

        // Subdividir en 4
        double half = current.h / 2.0;
        queue.push(Cell({ current.c.x - half, current.c.y - half }, half, poly));
        queue.push(Cell({ current.c.x + half, current.c.y - half }, half, poly));
        queue.push(Cell({ current.c.x - half, current.c.y + half }, half, poly));
        queue.push(Cell({ current.c.x + half, current.c.y + half }, half, poly));
    }

    return best.c;
}