#pragma once

#include "raylib.h"

// Mouse state for one frame. Game objects take this instead of polling raylib themselves.
struct Pointer {
    Vector2 pos{0, 0};
    bool pressed = false;    // went down this frame
    bool down = false;
    bool released = false;   // went up this frame
};

inline Pointer ReadPointer() {
    Pointer p;
    p.pos = GetMousePosition();
    p.pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    p.down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    p.released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    return p;
}
