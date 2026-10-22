#pragma once

#include "app/app_context.h"
#include "app/scene.h"
#include "game/feedback.h"
#include "game/level.h"
#include "game/slingshot.h"
#include "ui/button.h"
#include "ui/choice_dialog.h"
#include <cstddef>

// One round: a limited number of balls to fire at a level's targets
class PlayScene : public Scene {
public:
    PlayScene(AppContext& context, std::size_t levelIndex);
    ~PlayScene() override;

    void Update(float dt) override;
    void Draw() const override;

private:
    enum class Phase { Playing, Paused, Won, Lost };

    void UpdatePlaying(float dt, const Pointer& pointer);
    void UpdateRules(float dt);
    void Fire(const game::Slingshot::Launch& launch);
    void Pause();
    void Resume();
    void Finish(Phase result);
    void HandleChoice(ui::ChoiceDialog::Action action);
    void LeaveToLevels();

    game::BallKind nextBall() const;
    bool hasNextLevel() const;
    Camera2D ShakeCamera() const;
    void DrawHud() const;
    void DrawAmmoQueue() const;

    AppContext& context_;
    std::size_t levelIndex_;
    game::Level level_;   // before slingshot_, which is built from it
    game::Slingshot slingshot_;
    game::Feedback feedback_;
    ui::Button levelsButton_;
    ui::ChoiceDialog dialog_;

    Phase phase_ = Phase::Playing;
    int shotsLeft_;
    float winTimer_ = 0.0f;         // time since the last target fell
    float sinceLastShot_ = 0.0f;    // only counted once the balls have run out
};
