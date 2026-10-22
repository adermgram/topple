#include "ui/result_banner.h"
#include "config.h"
#include "game/rating.h"
#include "ui/star.h"
#include <algorithm>
#include <utility>

namespace ui {

namespace {

constexpr float PANEL_W = 520.0f;
constexpr float PANEL_H = 360.0f;
constexpr float PANEL_MARGIN = 36.0f;
constexpr float BUTTON_H = 58.0f;
constexpr float BUTTON_GAP = 14.0f;
constexpr float FADE_SECONDS = 0.35f;
constexpr float CLICKABLE_AFTER = 0.6f;   // fade progress before buttons react, so a late click can't skip it

constexpr float STAR_RADIUS = 28.0f;
constexpr float STAR_SPACING = 70.0f;
constexpr float STAR_FIRST_DELAY = 0.4f;  // seconds after the banner appears
constexpr float STAR_STAGGER = 0.22f;
constexpr float STAR_POP_TIME = 0.3f;

const Rectangle PANEL{(config::SCREEN_W - PANEL_W) / 2, (config::SCREEN_H - PANEL_H) / 2,
                      PANEL_W, PANEL_H};

const Color STAR_COLOR{255, 205, 90, 255};

// Overshoots slightly before settling, so a star "pops"
float EaseOutBack(float t) {
    const float c1 = 1.70158f, c3 = c1 + 1.0f;
    float u = t - 1.0f;
    return 1.0f + c3 * u * u * u + c1 * u * u;
}

}  // namespace

void ResultBanner::Show(std::string title, std::string subtitle, Color accent,
                        const std::vector<Choice>& choices, int stars) {
    title_ = std::move(title);
    subtitle_ = std::move(subtitle);
    accent_ = accent;
    stars_ = stars;
    visible_ = true;
    appear_ = 0.0f;
    shown_ = 0.0f;

    // Buttons share the panel's inner width equally
    const float count = (float)choices.size();
    const float width = (PANEL_W - 2 * PANEL_MARGIN - BUTTON_GAP * (count - 1)) / count;
    const float y = PANEL.y + PANEL_H - BUTTON_H - PANEL_MARGIN;

    entries_.clear();
    for (size_t i = 0; i < choices.size(); i++) {
        Rectangle bounds{PANEL.x + PANEL_MARGIN + i * (width + BUTTON_GAP), y, width, BUTTON_H};
        entries_.push_back({choices[i].action, Button(choices[i].label, bounds, choices[i].primary)});
    }
}

ResultBanner::Action ResultBanner::Update(float dt) {
    if (!visible_) return Action::None;
    shown_ += dt;
    appear_ = std::min(1.0f, appear_ + dt / FADE_SECONDS);

    Action chosen = Action::None;
    for (Entry& entry : entries_)
        if (entry.button.Update(dt) && appear_ >= CLICKABLE_AFTER) chosen = entry.action;
    return chosen;
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

    const float cx = PANEL.x + PANEL_W / 2;
    theme.DrawText(title_, {cx, PANEL.y + 40}, 52, Fade(accent_, appear_), Align::Center,
                   FontWeight::SemiBold);

    float subtitleY = PANEL.y + 125;
    if (stars_ != NO_STARS) {
        const float starY = PANEL.y + 135;
        for (int i = 0; i < game::MAX_STARS; i++) {
            Vector2 center = {cx + (i - (game::MAX_STARS - 1) / 2.0f) * STAR_SPACING, starY};
            if (i >= stars_) {
                DrawStar(center, STAR_RADIUS, false, Fade(p.textDim, 0.5f * appear_));
                continue;
            }
            float t = std::clamp((shown_ - STAR_FIRST_DELAY - i * STAR_STAGGER) / STAR_POP_TIME, 0.0f, 1.0f);
            if (t > 0.0f) DrawStar(center, STAR_RADIUS * EaseOutBack(t), true, STAR_COLOR);
        }
        subtitleY = PANEL.y + 190;
    }
    theme.DrawText(subtitle_, {cx, subtitleY}, 24, Fade(p.textDim, appear_), Align::Center);

    for (const Entry& entry : entries_) entry.button.Draw(theme);
}

}  // namespace ui
