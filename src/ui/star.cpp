#include "ui/star.h"
#include <cmath>

namespace ui {

namespace {

constexpr int POINTS = 10;              // 5 outer tips alternating with 5 inner corners
constexpr float INNER_RATIO = 0.42f;    // inner radius relative to the outer one

}  // namespace

void DrawStar(Vector2 center, float outerRadius, bool filled, Color color) {
    Vector2 corner[POINTS];
    for (int i = 0; i < POINTS; i++) {
        float angle = -PI / 2 + i * (2 * PI / POINTS);   // first tip points up
        float radius = (i % 2 == 0) ? outerRadius : outerRadius * INNER_RATIO;
        corner[i] = {center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius};
    }

    for (int i = 0; i < POINTS; i++) {
        Vector2 a = corner[i], b = corner[(i + 1) % POINTS];
        if (filled) {
            // raylib only draws triangles wound one way; submit both so the order never matters
            DrawTriangle(center, a, b, color);
            DrawTriangle(center, b, a, color);
        } else {
            DrawLineEx(a, b, 2.0f, color);
        }
    }
}

}  // namespace ui
