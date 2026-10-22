#pragma once

#include "app/app_context.h"
#include "app/scene.h"
#include "ui/button.h"
#include <vector>

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
