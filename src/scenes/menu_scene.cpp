#include "scenes/menu_scene.h"
#include "config.h"
#include "scenes/play_scene.h"
#include <memory>

namespace {

constexpr float BUTTON_W = 260.0f;
constexpr float BUTTON_H = 62.0f;
constexpr float BUTTON_GAP = 18.0f;

Rectangle CenteredButton(float y) {
    return {(config::SCREEN_W - BUTTON_W) / 2, y, BUTTON_W, BUTTON_H};
}

}  // namespace

MenuScene::MenuScene(AppContext& context)
    : context_(context),
      playButton_("Play", CenteredButton(360), true),
      quitButton_("Quit", CenteredButton(360 + BUTTON_H + BUTTON_GAP)) {}

void MenuScene::Update(float dt) {
    if (playButton_.Update(dt)) context_.scenes.Replace(std::make_unique<PlayScene>(context_));
    if (quitButton_.Update(dt)) context_.scenes.Quit();
}

void MenuScene::Draw() const {
    const ui::Theme& theme = context_.theme;
    const ui::Palette& p = theme.palette();

    theme.DrawBackground();

    float cx = config::SCREEN_W / 2.0f;
    theme.DrawText("TOPPLE", {cx, 150}, 110, p.accent, ui::Align::Center, ui::FontWeight::SemiBold);
    theme.DrawText("A physics puzzle", {cx, 275}, 30, p.textDim, ui::Align::Center);

    playButton_.Draw(theme);
    quitButton_.Draw(theme);
}
