#include "scenes/play_scene.h"
#include "app/scene_manager.h"
#include "audio/sound_bank.h"
#include "config.h"
#include "game/level_def.h"
#include "game/progress.h"
#include "game/rating.h"
#include "scenes/level_select_scene.h"
#include "ui/theme.h"
#include <algorithm>
#include <memory>
#include <string>

namespace {

    constexpr float WIN_DELAY = 0.8f;       // lets the last burst play before the dialog
    constexpr float LOSE_MIN_WAIT = 2.0f;   // after the last ball, give it time to land
    constexpr float LOSE_MAX_WAIT = 8.0f;   // ...but don't wait forever for everything to stop

    const Color LOSE_COLOR{232, 100, 90, 255};

}  // namespace

PlayScene::PlayScene(AppContext& context, std::size_t levelIndex)
    : context_(context),
      levelIndex_(levelIndex),
      title_("Level " + std::to_string(levelIndex + 1) + ": " + game::Levels().at(levelIndex).name),
      level_(game::Levels().at(levelIndex)),
      slingshot_(level_.slingAnchor(), level_.groundY()),
      feedback_(context.sound),
      levelsButton_("Levels", {20, 20, 120, 46}),
      shotsLeft_(level_.shots()) {}

PlayScene::~PlayScene() { SetMouseCursor(MOUSE_CURSOR_DEFAULT); }

bool PlayScene::hasNextLevel() const { return levelIndex_ + 1 < game::Levels().size(); }

game::BallKind PlayScene::nextBall() const { return level_.ammo(level_.shots() - shotsLeft_); }

void PlayScene::LeaveToLevels() {
    context_.scenes.Replace(std::make_unique<LevelSelectScene>(context_));
}

void PlayScene::Update(float dt) {
    const Pointer pointer = ReadPointer();
    const bool escape = IsKeyPressed(KEY_ESCAPE);

    switch (phase_) {
        case Phase::Playing:
            if (escape) {
                context_.sound.Play(audio::Sfx::Click);
                Pause();
            } else if (levelsButton_.Update(dt, pointer)) {
                context_.sound.Play(audio::Sfx::Click);
                LeaveToLevels();
                return;
            } else {
                UpdatePlaying(dt, pointer);
            }
            break;

        case Phase::Paused:
            if (escape) {
                context_.sound.Play(audio::Sfx::Click);
                Resume();
            }
            break;

        case Phase::Won:
        case Phase::Lost:
            if (escape) {
                LeaveToLevels();
                return;
            }
            feedback_.React(level_.Update(dt), dt);   // lets the last pieces and particles finish
            break;
        }

        HandleChoice(dialog_.Update(dt, pointer));
}

void PlayScene::UpdatePlaying(float dt, const Pointer& pointer) {
    slingshot_.setEnabled(shotsLeft_ > 0);
    if (shotsLeft_ > 0) slingshot_.setBall(nextBall());

    // The sling shakes with the world, so the mouse is mapped into it
    Pointer aim = pointer;
    aim.pos = GetScreenToWorld2D(pointer.pos, ShakeCamera());

    // A click while a splitter is flying splits it, rather than grabbing the sling
    if (aim.pressed && level_.TriggerAbility()) {
        context_.sound.Play(audio::Sfx::Split);
        aim.pressed = false;
    }
    if (auto launch = slingshot_.Update(dt, aim)) Fire(*launch);
    SetMouseCursor(slingshot_.wantsPointer() ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

    feedback_.React(level_.Update(dt), dt);
    UpdateRules(dt);
}

void PlayScene::UpdateRules(float dt) {
    if (level_.targetsRemaining() == 0) {
        winTimer_ += dt;
        if (winTimer_ >= WIN_DELAY) Finish(Phase::Won);
    } else if (shotsLeft_ == 0) {
        sinceLastShot_ += dt;
        const bool settled = sinceLastShot_ >= LOSE_MIN_WAIT && level_.isSettled();
        if (settled || sinceLastShot_ >= LOSE_MAX_WAIT) Finish(Phase::Lost);
    }
}

void PlayScene::Fire(const game::Slingshot::Launch& launch) {
    level_.Fire(launch.position, launch.velocity, nextBall());
    context_.sound.Play(audio::Sfx::Launch);
    shotsLeft_--;
    sinceLastShot_ = 0.0f;
}

void PlayScene::Pause() {
    using Action = ui::ChoiceDialog::Action;

    phase_ = Phase::Paused;
    feedback_.Reset();
    slingshot_.Cancel();   // otherwise the release happens while paused and is lost
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    dialog_.Show("PAUSED", title_, context_.theme.palette().text,
                 {{Action::Resume, "Resume", true}, {Action::Retry, "Retry", false}, {Action::Levels, "Levels", false}});
}

void PlayScene::Resume() {
    phase_ = Phase::Playing;
    dialog_.Hide();
}

void PlayScene::Finish(Phase result) {
    using Action = ui::ChoiceDialog::Action;

    phase_ = result;
    slingshot_.setEnabled(false);
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    context_.sound.Play(result == Phase::Won ? audio::Sfx::Win : audio::Sfx::Lose);

    if (result == Phase::Lost) {
        dialog_.Show("OUT OF SHOTS", std::to_string(level_.targetsRemaining()) + " targets survived", LOSE_COLOR,
                     {{Action::Retry, "Retry", true}, {Action::Levels, "Levels", false}});
        return;
    }

    const int used = level_.shots() - shotsLeft_;
    const int stars = game::StarsForShotsLeft(shotsLeft_);
    context_.progress.Record(levelIndex_, stars);

    const std::string subtitle = "Done in " + std::to_string(used) + (used == 1 ? " shot" : " shots");
    const Color accent = context_.theme.palette().accent;
    if (hasNextLevel())
        dialog_.Show("LEVEL CLEARED", subtitle, accent,
                     {{Action::Next, "Next", true}, {Action::Retry, "Retry", false}, {Action::Levels, "Levels", false}},
                     stars);
    else
        dialog_.Show("ALL CLEARED", subtitle, accent,
                     {{Action::Retry, "Retry", false}, {Action::Levels, "Levels", true}}, stars);
}

void PlayScene::HandleChoice(ui::ChoiceDialog::Action action) {
    using Action = ui::ChoiceDialog::Action;
    if (action != Action::None) context_.sound.Play(audio::Sfx::Click);

    switch (action) {
        case Action::Retry:
            context_.scenes.Replace(std::make_unique<PlayScene>(context_, levelIndex_));
            break;
        case Action::Next:
            context_.scenes.Replace(std::make_unique<PlayScene>(context_, levelIndex_ + 1));
            break;
        case Action::Levels:
            LeaveToLevels();
            break;
        case Action::Resume:
            Resume();
            break;
        case Action::None:
            break;
    }
}

// The world is drawn through this camera, the HUD and dialogs are not
Camera2D PlayScene::ShakeCamera() const {
    const Vector2 centre = {config::SCREEN_W / 2.0f, config::SCREEN_H / 2.0f};
    const Vector2 shake = feedback_.shake().offset();

    Camera2D camera{};
    camera.target = centre;
    camera.offset = {centre.x + shake.x, centre.y + shake.y};
    camera.rotation = feedback_.shake().rotationDegrees();
    camera.zoom = 1.0f;
    return camera;
}

void PlayScene::DrawAmmoQueue() const {
    const ui::Palette& p = context_.theme.palette();
    const int total = level_.shots();
    const int used = total - shotsLeft_;

    for (int i = 0; i < total; i++) {
        const game::BallKind kind = level_.ammo(i);
        const float fromRight = static_cast<float>(total - 1 - i);
        const Vector2 pos = {config::SCREEN_W - 36.0f - fromRight * 34.0f, 42.0f};
        const float radius = std::clamp(game::Ball::RadiusOf(kind) * 0.7f, 8.0f, 13.0f);

        if (i < used) DrawCircleLinesV(pos, radius, Fade(p.textDim, 0.5f));
        else DrawCircleV(pos, radius, game::Ball::ColorOf(kind));
        if (i == used && phase_ == Phase::Playing) DrawCircleLinesV(pos, radius + 4.0f, p.text);
    }
}

void PlayScene::DrawHud() const {
    const ui::Theme& theme = context_.theme;
    const ui::Palette& p = theme.palette();
    const float cx = config::SCREEN_W / 2.0f;

    DrawAmmoQueue();
    theme.DrawText("Targets left: " + std::to_string(level_.targetsRemaining()),
                   {config::SCREEN_W - 20.0f, 70}, 22, p.textDim, ui::Align::Right);
    theme.DrawText(title_, {cx, 22}, 26, p.text, ui::Align::Center, ui::FontWeight::SemiBold);

    if (phase_ != Phase::Playing) return;

    if (level_.canTriggerAbility())
        theme.DrawText("Click to split!", {cx, 60}, 22, p.accent, ui::Align::Center, ui::FontWeight::SemiBold);
    else if (levelIndex_ == 0 && shotsLeft_ == level_.shots())
        theme.DrawText("Drag the ball back and release to fire", {cx, 60}, 20, p.textDim, ui::Align::Center);

    if (shotsLeft_ > 0 && nextBall() != game::BallKind::Standard) {
        const Vector2 anchor = level_.slingAnchor();
        std::string label = game::Ball::NameOf(nextBall());
        if (nextBall() == game::BallKind::Splitter) label += " (click again in flight)";
        theme.DrawText(label, {anchor.x, anchor.y - 62.0f}, 20, p.textDim, ui::Align::Center);
    }
}

void PlayScene::Draw() const {
    const ui::Theme& theme = context_.theme;

    theme.DrawBackground();
    BeginMode2D(ShakeCamera());
        slingshot_.Draw();
        level_.Draw();
    EndMode2D();

    levelsButton_.Draw(theme);
    DrawHud();
    dialog_.Draw(theme);
}
