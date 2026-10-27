#include "ui/button.h"
#include "util/color.h"
#include <cmath>
#include <utility>

namespace ui {

    namespace {

        constexpr float HOVER_SPEED = 14.0f;
        constexpr float ROUNDNESS = 0.35f;

    }  // namespace

    Button::Button(std::string label, Rectangle bounds, bool primary, float fontSize)
        : label_(std::move(label)), bounds_(bounds), primary_(primary), fontSize_(fontSize) {}

    bool Button::Update(float dt, const Pointer& pointer) {
        const bool over = CheckCollisionPointRec(pointer.pos, bounds_);

        // Exponential easing, so the animation speed doesn't depend on the frame rate
        const float target = over ? 1.0f : 0.0f;
        hover_ += (target - hover_) * (1.0f - std::exp(-HOVER_SPEED * dt));

        if (over && pointer.pressed) pressed_ = true;
        if (!pointer.released) return false;

        const bool clicked = pressed_ && over;
        pressed_ = false;
        return clicked;
    }

    void Button::Draw(const Theme& theme) const {
        const Palette& p = theme.palette();

        const float dip = pressed_ ? 2.0f : 0.0f;
        Rectangle body = bounds_;
        body.y += dip;

        Rectangle shadow = bounds_;
        shadow.y += 5.0f - dip;
        DrawRectangleRounded(shadow, ROUNDNESS, 12, p.shadow);

        const Color base = primary_ ? p.accent : p.button;
        const Color lit = primary_ ? util::Mix(p.accent, WHITE, 0.25f) : p.buttonHover;
        DrawRectangleRounded(body, ROUNDNESS, 12, util::Mix(base, lit, hover_));

        const Color textColor = primary_ ? Color{30, 24, 14, 255} : p.text;
        const Vector2 center = {body.x + body.width / 2, body.y + body.height / 2 - fontSize_ * 0.6f};
        theme.DrawText(label_, center, fontSize_, textColor, Align::Center, FontWeight::SemiBold);
    }

}  // namespace ui
