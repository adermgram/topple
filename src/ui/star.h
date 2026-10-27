#pragma once

#include "raylib.h"

namespace ui {

    inline constexpr Color STAR_COLOR{255, 205, 90, 255};

    void DrawStar(Vector2 center, float outerRadius, bool filled, Color color);

}  // namespace ui
