#include "scenes/level_select_scene.h"
#include "config.h"
#include "game/level_def.h"
#include "game/rating.h"
#include "scenes/menu_scene.h"
#include "scenes/play_scene.h"
#include "ui/star.h"
#include <algorithm>
#include <memory>
#include <string>

namespace {

constexpr int COLUMNS = 5;
constexpr float CARD_SIZE = 140.0f;
constexpr float CARD_GAP_X = 36.0f;
constexpr float CARD_GAP_Y = 100.0f;  // room for the stars and the name under each card
constexpr float GRID_TOP = 230.0f;
constexpr float STAR_RADIUS = 10.0f;
constexpr float STAR_SPACING = 26.0f;

const Color STAR_COLOR{255, 205, 90, 255};

Rectangle CardBounds(size_t index, size_t total) {
    size_t columnsInRow = std::min<size_t>(COLUMNS, total - (index / COLUMNS) * COLUMNS);
    float rowWidth = columnsInRow * CARD_SIZE + (columnsInRow - 1) * CARD_GAP_X;
    float x = (config::SCREEN_W - rowWidth) / 2 + (index % COLUMNS) * (CARD_SIZE + CARD_GAP_X);
    float y = GRID_TOP + (index / COLUMNS) * (CARD_SIZE + CARD_GAP_Y);
    return {x, y, CARD_SIZE, CARD_SIZE};
}

Rectangle BackBounds() { return {(config::SCREEN_W - 200.0f) / 2, config::SCREEN_H - 120.0f, 200.0f, 56.0f}; }

}  // namespace

LevelSelectScene::LevelSelectScene(AppContext& context)
    : context_(context), backButton_("Back", BackBounds()) {
    const auto& levels = game::Levels();
    for (size_t i = 0; i < levels.size(); i++)
        levelButtons_.emplace_back(std::to_string(i + 1), CardBounds(i, levels.size()), false, 52.0f);
}

void LevelSelectScene::Update(float dt) {
    bool back = IsKeyPressed(KEY_ESCAPE);
    if (backButton_.Update(dt)) {
        context_.sound.Play(audio::Sfx::Click);
        back = true;
    }
    if (back) {
        context_.scenes.Replace(std::make_unique<MenuScene>(context_));
        return;
    }
    for (size_t i = 0; i < levelButtons_.size(); i++) {
        if (levelButtons_[i].Update(dt)) {
            context_.sound.Play(audio::Sfx::Click);
            context_.scenes.Replace(std::make_unique<PlayScene>(context_, (int)i));
            return;
        }
    }
}

void LevelSelectScene::Draw() const {
    const ui::Theme& theme = context_.theme;
    const ui::Palette& p = theme.palette();

    theme.DrawBackground();
    theme.DrawText("Select Level", {config::SCREEN_W / 2.0f, 90}, 64, p.accent, ui::Align::Center,
                   ui::FontWeight::SemiBold);

    const int totalPossible = (int)levelButtons_.size() * game::MAX_STARS;
    theme.DrawText("Stars: " + std::to_string(context_.progress.totalStars()) + " / " +
                       std::to_string(totalPossible),
                   {config::SCREEN_W / 2.0f, 170}, 22, p.textDim, ui::Align::Center);

    const auto& levels = game::Levels();
    for (size_t i = 0; i < levelButtons_.size(); i++) {
        levelButtons_[i].Draw(theme);
        Rectangle card = CardBounds(i, levels.size());
        float cx = card.x + card.width / 2;

        int earned = context_.progress.stars(i);
        for (int s = 0; s < game::MAX_STARS; s++) {
            Vector2 center = {cx + (s - (game::MAX_STARS - 1) / 2.0f) * STAR_SPACING,
                              card.y + card.height + 24};
            if (s < earned) ui::DrawStar(center, STAR_RADIUS, true, STAR_COLOR);
            else ui::DrawStar(center, STAR_RADIUS, false, Fade(p.textDim, 0.5f));
        }
        theme.DrawText(levels[i].name, {cx, card.y + card.height + 48}, 20, p.textDim,
                       ui::Align::Center);
    }
    backButton_.Draw(theme);
}
