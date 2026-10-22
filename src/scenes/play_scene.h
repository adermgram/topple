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
    explicit PlayScene(AppContext& context);
    ~PlayScene() override;

    void Update(float dt) override;
    void Draw() const override;

private:
    enum class Phase { Playing, Won, Lost };

    void Fire(const game::Slingshot::Launch& launch);
    void UpdateRules(float dt);
    void Finish(Phase result);
    void HandleBanner(ui::ResultBanner::Action action);
    void DrawHud() const;

    AppContext& context_;
    game::Level level_;               // declared before slingshot_, which is built from it
    game::Slingshot slingshot_;
    ui::Button menuButton_;
    ui::ResultBanner banner_;

    Phase phase_ = Phase::Playing;
    int shotsLeft_;
    float winTimer_ = 0.0f;           // time since the last target fell
    float sinceLastShot_ = 0.0f;      // counted only once the shots have run out
};
