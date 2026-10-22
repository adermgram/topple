#include "ui/result_banner.h"
#include "config.h"
#include <algorithm>
#include <utility>

namespace ui {

namespace {

constexpr float PANEL_W = 520.0f;
constexpr float PANEL_H = 320.0f;
constexpr float BUTTON_W = 200.0f;
constexpr float BUTTON_H = 58.0f;
constexpr float FADE_SECONDS = 0.35f;
constexpr float CLICKABLE_AFTER = 0.6f;   // fade progress before buttons react, so a late click can't skip it

const Rectangle PANEL{(config::SCREEN_W - PANEL_W) / 2, (config::SCREEN_H - PANEL_H) / 2,
                      PANEL_W, PANEL_H};

}  // namespace

ResultBanner::ResultBanner()
    : retryButton_("Retry", {PANEL.x + 40, PANEL.y + PANEL_H - BUTTON_H - 40, BUTTON_W, BUTTON_H}, true),
      menuButton_("Menu", {PANEL.x + PANEL_W - BUTTON_W - 40, PANEL.y + PANEL_H - BUTTON_H - 40,
                           BUTTON_W, BUTTON_H}) {}

void ResultBanner::Show(std::string title, std::string subtitle, Color accent) {
    title_ = std::move(title);
    subtitle_ = std::move(subtitle);
    accent_ = accent;
    visible_ = true;
    appear_ = 0.0f;
}

ResultBanner::Action ResultBanner::Update(float dt) {
    if (!visible_) return Action::None;
    appear_ = std::min(1.0f, appear_ + dt / FADE_SECONDS);

    bool retry = retryButton_.Update(dt);
    bool menu = menuButton_.Update(dt);
    if (appear_ < CLICKABLE_AFTER) return Action::None;
    if (retry) return Action::Retry;
    if (menu) return Action::Menu;
    return Action::None;
}

void ResultBanner::Draw(const Theme& theme) const {
    if (!visible_) return;
    const Palette& p = theme.palette();

    DrawRectangle(0, 0, config::SCREEN_W, config::SCREEN_H, Fade(BLACK, 0.55f * appear_));

    Rectangle shadow = PANEL;
    shadow.y += 8;
    DrawRectangleRounded(shadow, 0.08f, 12, Fade(BLACK, 0.4f * appear_));
    DrawRectangleRounded(PANEL, 0.08f, 12, Fade(p.panel, appear_));
    DrawRectangleRoundedLines(PANEL, 0.08f, 12, 2.0f, Fade(accent_, 0.6f * appear_));

    float cx = PANEL.x + PANEL_W / 2;
    theme.DrawText(title_, {cx, PANEL.y + 50}, 52, Fade(accent_, appear_), Align::Center,
                   FontWeight::SemiBold);
    theme.DrawText(subtitle_, {cx, PANEL.y + 125}, 24, Fade(p.textDim, appear_), Align::Center);

    retryButton_.Draw(theme);
    menuButton_.Draw(theme);
}

}  // namespace ui
