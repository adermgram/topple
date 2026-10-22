#pragma once

#include "raylib.h"
#include "raymath.h"

namespace util {

inline Color Shade(Color c, float factor) {
    return {static_cast<unsigned char>(c.r * factor), static_cast<unsigned char>(c.g * factor),
            static_cast<unsigned char>(c.b * factor), c.a};
}

inline Color Mix(Color a, Color b, float t) {
    return {static_cast<unsigned char>(Lerp(a.r, b.r, t)), static_cast<unsigned char>(Lerp(a.g, b.g, t)),
            static_cast<unsigned char>(Lerp(a.b, b.b, t)), static_cast<unsigned char>(Lerp(a.a, b.a, t))};
}

}  // namespace util
