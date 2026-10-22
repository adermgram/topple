#include "ui/button.h"
#include "raymath.h"
#include <cmath>
#include <utility>

namespace ui {

namespace {

constexpr float HOVER_SPEED = 14.0f;
constexpr float ROUNDNESS = 0.35f;
constexpr float FONT_SIZE = 26.0f;

Color Mix(Color a, Color b, float t) {
    return {(unsigned char)Lerp(a.r, b.r, t), (unsigned char)Lerp(a.g, b.g, t),
            (unsigned char)Lerp(a.b, b.b, t), (unsigned char)Lerp(a.a, b.a, t)};
}

}  // namespace

Button::Button(std::string label, Rectangle bounds, bool primary)
    : label_(std::move(label)), bounds_(bounds), primary_(primary) {}

bool Button::Update(float dt) {
    bool over = CheckCollisionPointRec(GetMousePosition(), bounds_);

    // Exponential ease so the animation speed doesn't depend on frame rate
    float target = over ? 1.0f : 0.0f;
    hover_ += (target - hover_) * (1.0f - std::exp(-HOVER_SPEED * dt));

    bool clicked = false;
    if (over && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) pressed_ = true;
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        clicked = pressed_ && over;
        pressed_ = false;
    }
    return clicked;
}

void Button::Draw(const Theme& theme) const {
    const Palette& p = theme.palette();

    float dip = pressed_ ? 2.0f : 0.0f;
    Rectangle body = bounds_;
    body.y += dip;

    // Soft shadow underneath, shrinking when the button is pressed
    Rectangle shadow = bounds_;
    shadow.y += 5.0f - dip;
    DrawRectangleRounded(shadow, ROUNDNESS, 12, p.shadow);

    Color base = primary_ ? p.accent : p.button;
    Color lit = primary_ ? Mix(p.accent, WHITE, 0.25f) : p.buttonHover;
    DrawRectangleRounded(body, ROUNDNESS, 12, Mix(base, lit, hover_));

    Color textColor = primary_ ? Color{30, 24, 14, 255} : p.text;
    Vector2 center = {body.x + body.width / 2, body.y + body.height / 2 - FONT_SIZE * 0.6f};
    theme.DrawText(label_, center, FONT_SIZE, textColor, Align::Center, FontWeight::SemiBold);
}

}  // namespace ui
