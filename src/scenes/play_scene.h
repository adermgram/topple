#pragma once

#include "app/app_context.h"
#include "app/scene.h"
#include "game/level.h"
#include "game/slingshot.h"
#include "ui/button.h"
#include "ui/result_banner.h"

// One round: the player fires a limited number of shots at the level's targets.
class PlayScene : public Scene {
public:
    PlayScene(AppContext& context, int levelIndex);
    ~PlayScene() override;

    void Update(float dt) override;
    void Draw() const override;

private:
    enum class Phase { Playing, Won, Lost };

    void Fire(const game::Slingshot::Launch& launch);
    void PlaySounds(const game::LevelEvents& events, float dt);
    void UpdateRules(float dt);
    void Finish(Phase result);
    void HandleBanner(ui::ResultBanner::Action action);
    bool HasNextLevel() const;
    void DrawHud() const;

    AppContext& context_;
    int levelIndex_;
    game::Level level_;               // declared before slingshot_, which is built from it
    game::Slingshot slingshot_;
    ui::Button levelsButton_;
    ui::ResultBanner banner_;

    Phase phase_ = Phase::Playing;
    int shotsLeft_;
    float winTimer_ = 0.0f;           // time since the last target fell
    float sinceLastShot_ = 0.0f;      // counted only once the shots have run out
    float hitCooldown_ = 0.0f;        // time until another hit sound may play
    float lastHitImpact_ = 0.0f;      // impulse of the hit that started the cooldown
};
