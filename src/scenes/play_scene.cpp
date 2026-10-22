#include "scenes/play_scene.h"
#include "app/pointer.h"
#include "config.h"
#include "game/level_def.h"
#include "game/rating.h"
#include "scenes/level_select_scene.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

namespace {

constexpr float WIN_DELAY = 0.8f;          // let the last burst play before the banner
constexpr float LOSE_MIN_WAIT = 2.0f;      // after the last shot, always give it time to land
constexpr float LOSE_MAX_WAIT = 8.0f;      // ...but don't wait forever for things to stop moving

// Collision impulses: pieces settling stay below ~6, a ball landing reaches 20-45
constexpr float HIT_SOUND_MIN_IMPACT = 8.0f;
constexpr float HIT_SOUND_FULL_VOLUME_AT = 40.0f;
constexpr float HIT_SOUND_MIN_VOLUME = 0.3f;
constexpr float HIT_SOUND_COOLDOWN = 0.08f;   // a pile-up shouldn't turn into a machine gun
constexpr float HIT_SOUND_BREAKTHROUGH = 1.5f; // ...but a hit this much harder than the last one is a new event
constexpr float HIT_SOUND_PITCH_VARIATION = 0.15f;

// Camera shake, calibrated on measured collisions: the hardest hit of a standard-ball shot is
// about 41 (up to ~65), of a heavy-ball shot about 146 (up to ~230). A typical standard hit
// gives trauma 0.5, a clear thump; the heavy ball reaches the cap, a real rattle.
constexpr float SHAKE_MIN_IMPACT = 15.0f;          // below this, nothing
constexpr float SHAKE_REFERENCE_IMPACT = 41.0f;
constexpr float SHAKE_REFERENCE_TRAUMA = 0.5f;
constexpr float SHAKE_EXPONENT = 0.6f;             // < 1: doubling the impact adds less than double
constexpr float SHAKE_MAX_FROM_IMPACT = 0.9f;
constexpr float SHAKE_PER_GLASS = 0.10f;
constexpr float SHAKE_PER_TARGET = 0.12f;

const Color WIN_COLOR{255, 184, 76, 255};
const Color LOSE_COLOR{232, 100, 90, 255};

}  // namespace

PlayScene::PlayScene(AppContext& context, int levelIndex)
    : context_(context),
      levelIndex_(levelIndex),
      level_(game::Levels().at(levelIndex)),
      slingshot_(level_.slingAnchor(), level_.groundY()),
      levelsButton_("Levels", {20, 20, 120, 46}),
      shotsLeft_(level_.shots()) {}

PlayScene::~PlayScene() { SetMouseCursor(MOUSE_CURSOR_DEFAULT); }

bool PlayScene::HasNextLevel() const {
    return levelIndex_ + 1 < (int)game::Levels().size();
}

// The ball that sits in the sling next (only meaningful while shots are left)
game::BallKind PlayScene::NextBall() const {
    return level_.ammo(level_.shots() - shotsLeft_);
}

void PlayScene::Fire(const game::Slingshot::Launch& launch) {
    level_.Fire(launch.position, launch.velocity, NextBall());
    context_.sound.Play(audio::Sfx::Launch);
    shotsLeft_--;
    sinceLastShot_ = 0.0f;
}

// The world (not the HUD) is drawn through this camera, nudged by the shake
Camera2D PlayScene::ShakeCamera() const {
    const Vector2 centre = {config::SCREEN_W / 2.0f, config::SCREEN_H / 2.0f};
    const Vector2 shake = shake_.offset();

    Camera2D camera{};
    camera.target = centre;
    camera.offset = {centre.x + shake.x, centre.y + shake.y};
    camera.rotation = shake_.rotationDegrees();
    camera.zoom = 1.0f;
    return camera;
}

void PlayScene::React(const game::LevelEvents& events, float dt) {
    PlaySounds(events, dt);

    // A hit lasts a few physics steps, so it raises the shake to a level; glass and targets
    // are one-off events, so they add to it
    if (events.strongestImpact >= SHAKE_MIN_IMPACT) {
        float trauma = SHAKE_REFERENCE_TRAUMA *
                       std::pow(events.strongestImpact / SHAKE_REFERENCE_IMPACT, SHAKE_EXPONENT);
        shake_.Raise(std::min(trauma, SHAKE_MAX_FROM_IMPACT));
    }
    shake_.AddTrauma(SHAKE_PER_GLASS * events.glassShattered + SHAKE_PER_TARGET * events.targetsDestroyed);
}

void PlayScene::PlaySounds(const game::LevelEvents& events, float dt) {
    hitCooldown_ -= dt;
    if (hitCooldown_ <= 0.0f) lastHitImpact_ = 0.0f;

    if (events.targetsDestroyed > 0) context_.sound.PlayVaried(audio::Sfx::Pop);
    if (events.glassShattered > 0) context_.sound.PlayVaried(audio::Sfx::Shatter);

    const float impact = events.strongestImpact;
    const bool coolingDown = hitCooldown_ > 0.0f;
    if (impact >= HIT_SOUND_MIN_IMPACT && (!coolingDown || impact > lastHitImpact_ * HIT_SOUND_BREAKTHROUGH)) {
        float volume = std::clamp(impact / HIT_SOUND_FULL_VOLUME_AT, HIT_SOUND_MIN_VOLUME, 1.0f);
        context_.sound.PlayVaried(audio::Sfx::Hit, volume, HIT_SOUND_PITCH_VARIATION);
        hitCooldown_ = HIT_SOUND_COOLDOWN;
        lastHitImpact_ = impact;
    }
}

void PlayScene::LeaveToLevels() {
    context_.scenes.Replace(std::make_unique<LevelSelectScene>(context_));
}

void PlayScene::Update(float dt) {
    const bool escape = IsKeyPressed(KEY_ESCAPE);
    if (phase_ != Phase::Paused) shake_.Update(dt);

    switch (phase_) {
    case Phase::Playing:
        if (escape) {
            context_.sound.Play(audio::Sfx::Click);
            Pause();
        } else if (levelsButton_.Update(dt)) {
            context_.sound.Play(audio::Sfx::Click);
            LeaveToLevels();
            return;
        } else {
            UpdatePlaying(dt);
        }
        break;

    case Phase::Paused:
        // Everything is frozen: only the dialog reacts
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
        React(level_.Update(dt), dt);   // let the last pieces and particles finish
        break;
    }

    HandleChoice(dialog_.Update(dt));
}

void PlayScene::UpdatePlaying(float dt) {
    slingshot_.set_enabled(shotsLeft_ > 0);
    if (shotsLeft_ > 0) slingshot_.set_ball(NextBall());

    // A click while a splitter is in the air splits it, instead of grabbing the sling
    Pointer pointer = ReadPointer();
    pointer.pos = GetScreenToWorld2D(pointer.pos, ShakeCamera());   // the sling moves with the shake
    if (pointer.pressed && level_.TriggerAbility()) {
        context_.sound.Play(audio::Sfx::Split);
        pointer.pressed = false;
    }
    if (auto launch = slingshot_.Update(dt, pointer)) Fire(*launch);
    SetMouseCursor(slingshot_.WantsPointer() ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

    React(level_.Update(dt), dt);
    UpdateRules(dt);
}

void PlayScene::Pause() {
    using Action = ui::ChoiceDialog::Action;

    phase_ = Phase::Paused;
    shake_.Reset();        // a frozen, half-shaken screen would look broken
    slingshot_.Cancel();   // a half-pulled ball goes back, so no release is missed while paused
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    dialog_.Show("PAUSED", "Level " + std::to_string(levelIndex_ + 1) + ": " + game::Levels()[levelIndex_].name,
                 context_.theme.palette().text,
                 {{Action::Resume, "Resume", true}, {Action::Retry, "Retry", false},
                  {Action::Levels, "Levels", false}});
}

void PlayScene::Resume() {
    phase_ = Phase::Playing;
    dialog_.Hide();
}

void PlayScene::UpdateRules(float dt) {
    if (level_.targetsRemaining() == 0) {
        winTimer_ += dt;
        if (winTimer_ >= WIN_DELAY) Finish(Phase::Won);
    } else if (shotsLeft_ == 0) {
        sinceLastShot_ += dt;
        bool settled = sinceLastShot_ >= LOSE_MIN_WAIT && level_.IsSettled();
        if (settled || sinceLastShot_ >= LOSE_MAX_WAIT) Finish(Phase::Lost);
    }
}

void PlayScene::Finish(Phase result) {
    using Action = ui::ChoiceDialog::Action;

    phase_ = result;
    slingshot_.set_enabled(false);
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    context_.sound.Play(result == Phase::Won ? audio::Sfx::Win : audio::Sfx::Lose);

    if (result == Phase::Won) {
        int used = level_.shots() - shotsLeft_;
        int stars = game::StarsForShotsLeft(shotsLeft_);
        context_.progress.Record(levelIndex_, stars);

        std::string subtitle = "Done in " + std::to_string(used) + (used == 1 ? " shot" : " shots");
        if (HasNextLevel())
            dialog_.Show("LEVEL CLEARED", subtitle, WIN_COLOR,
                         {{Action::Next, "Next", true}, {Action::Retry, "Retry", false},
                          {Action::Levels, "Levels", false}},
                         stars);
        else
            dialog_.Show("ALL CLEARED", subtitle, WIN_COLOR,
                         {{Action::Retry, "Retry", false}, {Action::Levels, "Levels", true}}, stars);
    } else {
        dialog_.Show("OUT OF SHOTS", std::to_string(level_.targetsRemaining()) + " targets survived",
                     LOSE_COLOR, {{Action::Retry, "Retry", true}, {Action::Levels, "Levels", false}});
    }
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

void PlayScene::DrawHud() const {
    const ui::Theme& theme = context_.theme;
    const ui::Palette& p = theme.palette();

    // The ammo queue, first ball on the left: each icon shows its kind, used ones are dimmed
    const int total = level_.shots();
    const int used = total - shotsLeft_;
    for (int i = 0; i < total; i++) {
        game::BallKind kind = level_.ammo(i);
        Vector2 pos = {config::SCREEN_W - 36.0f - (total - 1 - i) * 34.0f, 42.0f};
        float radius = std::clamp(game::Ball::RadiusOf(kind) * 0.7f, 8.0f, 13.0f);
        if (i < used) DrawCircleLinesV(pos, radius, Fade(p.textDim, 0.5f));
        else DrawCircleV(pos, radius, game::Ball::ColorOf(kind));
        if (i == used && phase_ == Phase::Playing) DrawCircleLinesV(pos, radius + 4.0f, p.text);   // up next
    }
    theme.DrawText("Targets left: " + std::to_string(level_.targetsRemaining()),
                   {config::SCREEN_W - 20.0f, 70}, 22, p.textDim, ui::Align::Right);

    const float cx = config::SCREEN_W / 2.0f;
    theme.DrawText("Level " + std::to_string(levelIndex_ + 1) + ": " + game::Levels()[levelIndex_].name,
                   {cx, 22}, 26, p.text, ui::Align::Center, ui::FontWeight::SemiBold);

    if (phase_ != Phase::Playing) return;

    if (level_.CanTriggerAbility())
        theme.DrawText("Click to split!", {cx, 60}, 22, p.accent, ui::Align::Center, ui::FontWeight::SemiBold);
    else if (levelIndex_ == 0 && shotsLeft_ == level_.shots())
        theme.DrawText("Drag the ball back and release to fire", {cx, 60}, 20, p.textDim,
                       ui::Align::Center);

    // Name the ball waiting in the sling when it isn't the plain one
    if (shotsLeft_ > 0 && NextBall() != game::BallKind::Standard) {
        Vector2 anchor = level_.slingAnchor();
        std::string label = game::Ball::NameOf(NextBall());
        if (NextBall() == game::BallKind::Splitter) label += " (click again in flight)";
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
