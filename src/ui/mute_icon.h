#pragma once

#include "raylib.h"

namespace ui {

    // A small speaker, with sound waves when sound is on and a red cross when it is muted
    void DrawMuteIcon(Vector2 center, bool muted);

}  // namespace ui
