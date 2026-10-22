#include "scenes/play_scene.h"
#include "app/pointer.h"
#include "config.h"
#include "game/level_def.h"
#include "game/rating.h"
#include "scenes/level_select_scene.h"
#include <algorithm>
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

void PlayScene::Fire(const game::Slingshot::Launch& launch) {
    level_.Fire(launch.position, launch.velocity);
    context_.sound.Play(audio::Sfx::Launch);
    shotsLeft_--;
    sinceLastShot_ = 0.0f;
}

void PlayScene::PlaySounds(const game::LevelEvents& events, float dt) {
    hitCooldown_ -= dt;
    if (hitCooldown_ <= 0.0f) lastHitImpact_ = 0.0f;

    if (events.targetsDestroyed > 0) context_.sound.PlayVaried(audio::Sfx::Pop);

    const float impact = events.strongestImpact;
    const bool coolingDown = hitCooldown_ > 0.0f;
    if (impact >= HIT_SOUND_MIN_IMPACT && (!coolingDown || impact > lastHitImpact_ * HIT_SOUND_BREAKTHROUGH)) {
        float volume = std::clamp(impact / HIT_SOUND_FULL_VOLUME_AT, HIT_SOUND_MIN_VOLUME, 1.0f);
        context_.sound.PlayVaried(audio::Sfx::Hit, volume, HIT_SOUND_PITCH_VARIATION);
        hitCooldown_ = HIT_SOUND_COOLDOWN;
        lastHitImpact_ = impact;
    }
}

void PlayScene::Update(float dt) {
    bool leave = IsKeyPressed(KEY_ESCAPE);
    if (!banner_.visible() && levelsButton_.Update(dt)) {
        context_.sound.Play(audio::Sfx::Click);
        leave = true;
    }
    if (leave) {
        context_.scenes.Replace(std::make_unique<LevelSelectScene>(context_));
        return;
    }

    if (phase_ == Phase::Playing) {
        slingshot_.set_enabled(shotsLeft_ > 0);
        if (auto launch = slingshot_.Update(dt, ReadPointer())) Fire(*launch);
        SetMouseCursor(slingshot_.WantsPointer() ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);
    }

    PlaySounds(level_.Update(dt), dt);
    if (phase_ == Phase::Playing) UpdateRules(dt);
    HandleBanner(banner_.Update(dt));
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
    using Action = ui::ResultBanner::Action;

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
            banner_.Show("LEVEL CLEARED", subtitle, WIN_COLOR,
                         {{Action::Next, "Next", true}, {Action::Retry, "Retry", false},
                          {Action::Levels, "Levels", false}},
                         stars);
        else
            banner_.Show("ALL CLEARED", subtitle, WIN_COLOR,
                         {{Action::Retry, "Retry", false}, {Action::Levels, "Levels", true}}, stars);
    } else {
        banner_.Show("OUT OF SHOTS", std::to_string(level_.targetsRemaining()) + " targets survived",
                     LOSE_COLOR, {{Action::Retry, "Retry", true}, {Action::Levels, "Levels", false}});
    }
}

void PlayScene::HandleBanner(ui::ResultBanner::Action action) {
    using Action = ui::ResultBanner::Action;
    if (action != Action::None) context_.sound.Play(audio::Sfx::Click);

    switch (action) {
    case Action::Retry:
        context_.scenes.Replace(std::make_unique<PlayScene>(context_, levelIndex_));
        break;
    case Action::Next:
        context_.scenes.Replace(std::make_unique<PlayScene>(context_, levelIndex_ + 1));
        break;
    case Action::Levels:
        context_.scenes.Replace(std::make_unique<LevelSelectScene>(context_));
        break;
    case Action::None:
        break;
    }
}

void PlayScene::DrawHud() const {
    const ui::Theme& theme = context_.theme;
    const ui::Palette& p = theme.palette();

    // One ball icon per shot, dimmed once used
    for (int i = 0; i < level_.shots(); i++) {
        Vector2 pos = {config::SCREEN_W - 36.0f - i * 34.0f, 42.0f};
        if (i < shotsLeft_) DrawCircleV(pos, 11.0f, game::Ball::COLOR);
        else DrawCircleLinesV(pos, 11.0f, Fade(p.textDim, 0.5f));
    }
    theme.DrawText("Targets left: " + std::to_string(level_.targetsRemaining()),
                   {config::SCREEN_W - 20.0f, 70}, 22, p.textDim, ui::Align::Right);

    const float cx = config::SCREEN_W / 2.0f;
    theme.DrawText("Level " + std::to_string(levelIndex_ + 1) + ": " + game::Levels()[levelIndex_].name,
                   {cx, 22}, 26, p.text, ui::Align::Center, ui::FontWeight::SemiBold);
    if (levelIndex_ == 0 && shotsLeft_ == level_.shots() && phase_ == Phase::Playing)
        theme.DrawText("Drag the ball back and release to fire", {cx, 60}, 20, p.textDim,
                       ui::Align::Center);
}

void PlayScene::Draw() const {
    const ui::Theme& theme = context_.theme;

    theme.DrawBackground();
    slingshot_.Draw();
    level_.Draw();

    levelsButton_.Draw(theme);
    DrawHud();
    banner_.Draw(theme);
}
