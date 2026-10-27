#pragma once

#include "raylib.h"

struct Pointer {
    Vector2 pos{0, 0};
    bool pressed = false;    
    bool released = false;
};

inline Pointer ReadPointer() {
    Pointer p;
    p.pos = GetMousePosition();
    p.pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    p.released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    return p;
}
