#include "app/app.h"
#include "config.h"
#include "raylib.h"
#include "scenes/menu_scene.h"
#include <algorithm>

App::WindowGuard::WindowGuard() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);   // smooth rounded shapes
    InitWindow(config::SCREEN_W, config::SCREEN_H, config::TITLE);
    SetTargetFPS(config::TARGET_FPS);
    SetExitKey(KEY_NULL);   // Esc is used by the scenes, not to close the window
}

App::WindowGuard::~WindowGuard() { CloseWindow(); }

App::AudioGuard::AudioGuard() { InitAudioDevice(); }
App::AudioGuard::~AudioGuard() { CloseAudioDevice(); }

// Progress is saved next to the executable, like the resources
App::App() : progress_(std::string(GetApplicationDirectory()) + "progress.txt") {
    scenes_.Replace(std::make_unique<MenuScene>(context_));
}

void App::Run() {
    while (!WindowShouldClose() && !scenes_.QuitRequested()) {
        if (IsKeyPressed(KEY_M)) sound_.ToggleMute();
        scenes_.Update(std::min(GetFrameTime(), config::MAX_FRAME_TIME));

        BeginDrawing();
            scenes_.Draw();
        EndDrawing();
    }
}
