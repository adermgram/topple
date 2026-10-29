#include "app/app.h"
#include "config.h"
#include "raylib.h"
#include "scenes/menu_scene.h"
#include "util/save_path.h"
#include <algorithm>
#include <memory>
#include <string>

App::WindowGuard::WindowGuard() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(config::SCREEN_W, config::SCREEN_H, config::TITLE);
    SetTargetFPS(config::TARGET_FPS);
    SetExitKey(KEY_NULL);   // Esc belongs to the scenes
}

App::WindowGuard::~WindowGuard() { CloseWindow(); }

App::AudioGuard::AudioGuard() { InitAudioDevice(); }
App::AudioGuard::~AudioGuard() { CloseAudioDevice(); }

App::App() : progress_(util::SaveFilePath("progress.txt", GetApplicationDirectory())) {
    scenes_.Replace(std::make_unique<MenuScene>(context_));
}

void App::Run() {
    while (!WindowShouldClose() && !scenes_.quitRequested()) {
        if (IsKeyPressed(KEY_M)) sound_.ToggleMute();
        scenes_.Update(std::min(GetFrameTime(), config::MAX_FRAME_TIME));

        BeginDrawing();
            scenes_.Draw();
        EndDrawing();
    }
}
