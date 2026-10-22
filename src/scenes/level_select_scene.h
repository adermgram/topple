#pragma once

#include "app/app_context.h"
#include "app/scene.h"
#include "ui/button.h"
#include <vector>

// A row of numbered cards, one per level, plus a way back to the main menu.
class LevelSelectScene : public Scene {
public:
    explicit LevelSelectScene(AppContext& context);

    void Update(float dt) override;
    void Draw() const override;

private:
    AppContext& context_;
    std::vector<ui::Button> levelButtons_;   // same order as game::Levels()
    ui::Button backButton_;
};
