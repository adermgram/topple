#include "app/app.h"
#include "config.h"
#include "raylib.h"
#include "rlgl.h"
#include "scenes/menu_scene.h"
#include "ui/mute_icon.h"
#include "util/save_path.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

namespace {

    // The canvas has this many pixels per game pixel; scaled down to the window it smooths edges
    // (the window's own multisampling doesn't reach a render texture)
    constexpr int SUPERSAMPLE = 2;

    constexpr Vector2 MUTE_ICON_AT = {config::SCREEN_W - 36.0f, config::SCREEN_H - 30.0f};

    // Where the canvas lands in the window: as large as fits, centred, with bars at the sides or top
    struct Placement {
        Rectangle dest;
        float scale;
    };

    Placement FitCanvasToWindow() {
        const float width = static_cast<float>(GetScreenWidth());
        const float height = static_cast<float>(GetScreenHeight());
        const float scale = std::min(width / config::SCREEN_W, height / config::SCREEN_H);
        const float w = config::SCREEN_W * scale;
        const float h = config::SCREEN_H * scale;
        return {{std::floor((width - w) / 2), std::floor((height - h) / 2), w, h}, scale};
    }

}  // namespace

App::WindowGuard::WindowGuard() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(config::SCREEN_W, config::SCREEN_H, config::TITLE);
    SetWindowMinSize(config::SCREEN_W / 2, config::SCREEN_H / 2);
    SetTargetFPS(config::TARGET_FPS);
    SetExitKey(KEY_NULL);   // Esc belongs to the scenes
}

App::WindowGuard::~WindowGuard() { CloseWindow(); }

App::CanvasGuard::CanvasGuard()
    : texture(LoadRenderTexture(config::SCREEN_W * SUPERSAMPLE, config::SCREEN_H * SUPERSAMPLE)) {
    SetTextureFilter(texture.texture, TEXTURE_FILTER_BILINEAR);
}

App::CanvasGuard::~CanvasGuard() { UnloadRenderTexture(texture); }

App::AudioGuard::AudioGuard() { InitAudioDevice(); }
App::AudioGuard::~AudioGuard() { CloseAudioDevice(); }

App::App() : progress_(util::SaveFilePath("progress.txt", GetApplicationDirectory())) {
    scenes_.Replace(std::make_unique<MenuScene>(context_));
}

void App::Run() {
    while (!WindowShouldClose() && !scenes_.quitRequested()) {
        if (IsKeyPressed(KEY_M)) sound_.ToggleMute();
        if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

        // The scenes work in canvas coordinates, so the mouse is mapped out of the window's
        const Placement placement = FitCanvasToWindow();
        SetMouseOffset(-static_cast<int>(placement.dest.x), -static_cast<int>(placement.dest.y));
        SetMouseScale(1.0f / placement.scale, 1.0f / placement.scale);

        scenes_.Update(std::min(GetFrameTime(), config::MAX_FRAME_TIME));

        BeginTextureMode(canvas_.texture);
            // The texture is larger than the game's coordinates: keep drawing in those
            rlMatrixMode(RL_PROJECTION);
            rlLoadIdentity();
            rlOrtho(0, config::SCREEN_W, config::SCREEN_H, 0, 0.0, 1.0);
            rlMatrixMode(RL_MODELVIEW);

            scenes_.Draw();
            ui::DrawMuteIcon(MUTE_ICON_AT, sound_.muted());
        EndTextureMode();

        BeginDrawing();
            // The bars around the canvas use the same gradient, so they blend into the game
            const ui::Palette& palette = theme_.palette();
            DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), palette.bgTop, palette.bgBottom);
            const Texture2D& canvas = canvas_.texture.texture;
            // A render texture is stored upside down, hence the negative height
            DrawTexturePro(canvas, {0, 0, static_cast<float>(canvas.width), -static_cast<float>(canvas.height)},
                           placement.dest, {0, 0}, 0.0f, WHITE);
        EndDrawing();
    }
}
