#pragma once

#include "app/app_context.h"
#include "app/scene.h"
#include "game/level.h"
#include "game/screen_shake.h"
#include "game/slingshot.h"
#include "ui/button.h"
#include "ui/choice_dialog.h"

// One round: the player fires a limited number of shots at the level's targets.
class PlayScene : public Scene {
public:
    PlayScene(AppContext& context, int levelIndex);
    ~PlayScene() override;

    void Update(float dt) override;
    void Draw() const override;

private:
    enum class Phase { Playing, Paused, Won, Lost };

    game::BallKind NextBall() const;
    void Fire(const game::Slingshot::Launch& launch);
    void React(const game::LevelEvents& events, float dt);   // sounds and shake
    void PlaySounds(const game::LevelEvents& events, float dt);
    Camera2D ShakeCamera() const;
    void UpdateRules(float dt);
    void Finish(Phase result);
    void UpdatePlaying(float dt);
    void LeaveToLevels();
    void Pause();
    void Resume();
    void HandleChoice(ui::ChoiceDialog::Action action);
    bool HasNextLevel() const;
    void DrawHud() const;

    AppContext& context_;
    int levelIndex_;
    game::Level level_;               // declared before slingshot_, which is built from it
    game::Slingshot slingshot_;
    game::ScreenShake shake_;
    ui::Button levelsButton_;
    ui::ChoiceDialog dialog_;

    Phase phase_ = Phase::Playing;
    int shotsLeft_;
    float winTimer_ = 0.0f;           // time since the last target fell
    float sinceLastShot_ = 0.0f;      // counted only once the shots have run out
    float hitCooldown_ = 0.0f;        // time until another hit sound may play
    float lastHitImpact_ = 0.0f;      // impulse of the hit that started the cooldown
};
