#pragma once

#include "app/app_context.h"
#include "app/scene.h"
#include "ui/button.h"

class MenuScene : public Scene {
public:
    explicit MenuScene(AppContext& context);

    void Update(float dt) override;
    void Draw() const override;

private:
    AppContext& context_;
    ui::Button playButton_;
    ui::Button quitButton_;
};
