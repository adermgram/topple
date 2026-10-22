#include "scenes/level_select_scene.h"
#include "app/scene_manager.h"
#include "audio/sound_bank.h"
#include "config.h"
#include "game/level_def.h"
#include "game/progress.h"
#include "game/rating.h"
#include "input/pointer.h"
#include "scenes/menu_scene.h"
#include "scenes/play_scene.h"
#include "ui/star.h"
#include "ui/theme.h"
#include <algorithm>
#include <memory>
#include <string>

namespace {

constexpr int COLUMNS = 5;
constexpr float CARD_SIZE = 120.0f;
constexpr float CARD_GAP_X = 40.0f;
constexpr float CARD_GAP_Y = 84.0f;   // room for the stars and the name under each card
constexpr float GRID_TOP = 205.0f;
constexpr float CARD_FONT_SIZE = 46.0f;
constexpr float STAR_RADIUS = 10.0f;
constexpr float STAR_SPACING = 26.0f;

// Every row is centred on its own, so a short last row sits in the middle
Rectangle CardBounds(int index, int total) {
    const int row = index / COLUMNS;
    const int column = index % COLUMNS;
    const int cardsInRow = std::min(COLUMNS, total - row * COLUMNS);

    const float rowWidth = static_cast<float>(cardsInRow) * CARD_SIZE + static_cast<float>(cardsInRow - 1) * CARD_GAP_X;
    const float x = (config::SCREEN_W - rowWidth) / 2 + static_cast<float>(column) * (CARD_SIZE + CARD_GAP_X);
    const float y = GRID_TOP + static_cast<float>(row) * (CARD_SIZE + CARD_GAP_Y);
    return {x, y, CARD_SIZE, CARD_SIZE};
}

Rectangle BackBounds() { return {(config::SCREEN_W - 200.0f) / 2, config::SCREEN_H - 96.0f, 200.0f, 56.0f}; }

}  // namespace

LevelSelectScene::LevelSelectScene(AppContext& context)
    : context_(context), backButton_("Back", BackBounds()) {
    const int total = static_cast<int>(game::Levels().size());
    for (int i = 0; i < total; i++)
        levelButtons_.emplace_back(std::to_string(i + 1), CardBounds(i, total), false, CARD_FONT_SIZE);
}

void LevelSelectScene::Update(float dt) {
    const Pointer pointer = ReadPointer();

    bool back = IsKeyPressed(KEY_ESCAPE);
    if (backButton_.Update(dt, pointer)) {
        context_.sound.Play(audio::Sfx::Click);
        back = true;
    }
    if (back) {
        context_.scenes.Replace(std::make_unique<MenuScene>(context_));
        return;
    }

    for (std::size_t i = 0; i < levelButtons_.size(); i++) {
        if (levelButtons_[i].Update(dt, pointer)) {
            context_.sound.Play(audio::Sfx::Click);
            context_.scenes.Replace(std::make_unique<PlayScene>(context_, i));
            return;
        }
    }
}

void LevelSelectScene::Draw() const {
    const ui::Theme& theme = context_.theme;
    const ui::Palette& p = theme.palette();
    const auto& levels = game::Levels();
    const int total = static_cast<int>(levels.size());
    const float cx = config::SCREEN_W / 2.0f;

    theme.DrawBackground();
    theme.DrawText("Select Level", {cx, 90}, 64, p.accent, ui::Align::Center, ui::FontWeight::SemiBold);
    theme.DrawText("Stars: " + std::to_string(context_.progress.totalStars()) + " / " +
                       std::to_string(total * game::MAX_STARS),
                   {cx, 170}, 22, p.textDim, ui::Align::Center);

    for (std::size_t i = 0; i < levelButtons_.size(); i++) {
        levelButtons_[i].Draw(theme);

        const Rectangle card = CardBounds(static_cast<int>(i), total);
        const float cardCenter = card.x + card.width / 2;
        const int earned = context_.progress.stars(i);
        for (int s = 0; s < game::MAX_STARS; s++) {
            const float offset = static_cast<float>(s) - static_cast<float>(game::MAX_STARS - 1) / 2.0f;
            const Vector2 center = {cardCenter + offset * STAR_SPACING, card.y + card.height + 24};
            ui::DrawStar(center, STAR_RADIUS, s < earned, s < earned ? ui::STAR_COLOR : Fade(p.textDim, 0.5f));
        }
        theme.DrawText(levels[i].name, {cardCenter, card.y + card.height + 48}, 20, p.textDim, ui::Align::Center);
    }
    backButton_.Draw(theme);
}
