#pragma once

namespace config {

    constexpr int SCREEN_W = 1280;
    constexpr int SCREEN_H = 720;
    constexpr const char* TITLE = "Topple";
    constexpr int TARGET_FPS = 60;

    constexpr float MAX_FRAME_TIME = 0.1f;   // a window drag or stall must not become one huge step

}  // namespace config
