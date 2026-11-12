#include "ui/mute_icon.h"
#include <initializer_list>

namespace ui {

    namespace {

        const Color ON_COLOR{150, 158, 182, 170};
        const Color MUTED_COLOR{232, 100, 90, 230};

        // Raylib culls triangles wound the wrong way, so draw both windings
        void Triangle(Vector2 a, Vector2 b, Vector2 c, Color color) {
            DrawTriangle(a, b, c, color);
            DrawTriangle(a, c, b, color);
        }

    }  // namespace

    void DrawMuteIcon(Vector2 c, bool muted) {
        const Color color = muted ? MUTED_COLOR : ON_COLOR;

        DrawRectangleRec({c.x - 11, c.y - 4, 6, 8}, color);
        const Vector2 top = {c.x - 5, c.y - 4}, bottom = {c.x - 5, c.y + 4};
        const Vector2 farTop = {c.x + 2, c.y - 10}, farBottom = {c.x + 2, c.y + 10};
        Triangle(top, farTop, farBottom, color);
        Triangle(top, farBottom, bottom, color);

        if (muted) {
            DrawLineEx({c.x + 6, c.y - 5}, {c.x + 16, c.y + 5}, 2.5f, color);
            DrawLineEx({c.x + 16, c.y - 5}, {c.x + 6, c.y + 5}, 2.5f, color);
            return;
        }
        for (float radius : {7.0f, 12.0f})
            DrawRing({c.x + 2, c.y}, radius, radius + 2.0f, -45.0f, 45.0f, 12, color);
    }

}  // namespace ui
