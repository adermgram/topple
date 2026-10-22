#include "game/target.h"
#include <algorithm>
#include <cmath>

namespace game {

namespace {

constexpr float MAX_HEALTH = 15.0f;
constexpr float MIN_DAMAGING_IMPULSE = 4.0f;   // below this (resting, light bumps) nothing happens
constexpr float FLASH_TIME = 0.15f;
constexpr float DAZED_TIME = 0.9f;
constexpr float SQUASH_AMOUNT = 0.14f;         // fraction of the radius, at the moment of impact

constexpr float BLINK_PERIOD = 3.2f;           // seconds between blinks
constexpr float BLINK_TIME = 0.12f;

const physics::Material TARGET_MATERIAL{1.0f, 0.6f, 0.1f};

const Color SICK_COLOR{205, 205, 110, 255};    // what a nearly-dead target turns toward
const Color OUTLINE{50, 110, 60, 255};
const Color INK{30, 40, 30, 255};

Vector2 Rotate(Vector2 v, float angle) {
    float c = std::cos(angle), s = std::sin(angle);
    return {v.x * c - v.y * s, v.x * s + v.y * c};
}

Color Mix(Color a, Color b, float t) {
    return {(unsigned char)(a.r + (b.r - a.r) * t), (unsigned char)(a.g + (b.g - a.g) * t),
            (unsigned char)(a.b + (b.b - a.b) * t), 255};
}

}  // namespace

Target::Target(physics::World& world, Vector2 centerPx)
    : body_(world.CreateCircle(centerPx, RADIUS, physics::BodyType::Dynamic, TARGET_MATERIAL)),
      health_(MAX_HEALTH),
      blinkOffset_(std::fmod(centerPx.x * 0.0137f + centerPx.y * 0.0071f, BLINK_PERIOD)) {}

Vector2 Target::position() const { return physics::ToPixels(body_->GetPosition()); }

void Target::ApplyImpact(float impulse) {
    if (impulse < MIN_DAMAGING_IMPULSE) return;
    health_ -= impulse - MIN_DAMAGING_IMPULSE;
    flash_ = FLASH_TIME;
    dazed_ = DAZED_TIME;
}

void Target::Update(float dt) {
    time_ += dt;
    flash_ = std::max(0.0f, flash_ - dt);
    dazed_ = std::max(0.0f, dazed_ - dt);
}

void Target::Draw() const {
    const Vector2 c = position();
    const float angle = body_->GetAngle();

    // A fresh hit squashes it for an instant and flashes it white; damage tints it sickly
    const float hit = flash_ / FLASH_TIME;   // 1 at the moment of impact, 0 after
    const float health = std::clamp(health_ / MAX_HEALTH, 0.0f, 1.0f);
    const Color fill = hit > 0.0f ? Color{235, 250, 235, 255} : Mix(SICK_COLOR, COLOR, health);
    const float radius = RADIUS * (1.0f + SQUASH_AMOUNT * hit);

    DrawCircleV(c, radius, fill);
    DrawCircleLinesV(c, radius, OUTLINE);

    // Features rotate with the body
    auto at = [&](float x, float y) {
        Vector2 o = Rotate({x, y}, angle);
        return Vector2{c.x + o.x, c.y + o.y};
    };
    auto line = [&](Vector2 from, Vector2 to) { DrawLineEx(at(from.x, from.y), at(to.x, to.y), 2.0f, INK); };

    const bool dazed = dazed_ > 0.0f;
    const bool blinking = std::fmod(time_ + blinkOffset_, BLINK_PERIOD) < BLINK_TIME;

    for (float side : {-6.5f, 6.5f}) {
        if (dazed) {                                    // X eyes
            line({side - 3.5f, -7.5f}, {side + 3.5f, -0.5f});
            line({side - 3.5f, -0.5f}, {side + 3.5f, -7.5f});
        } else if (blinking) {                          // closed
            line({side - 3.5f, -4.0f}, {side + 3.5f, -4.0f});
        } else {                                        // open
            DrawCircleV(at(side, -4.0f), 4.5f, WHITE);
            DrawCircleV(at(side + (side < 0 ? 0.5f : -0.5f), -4.0f), 2.2f, INK);
        }
    }

    if (dazed) {
        DrawCircleV(at(0.0f, 8.0f), 3.6f, INK);         // open mouth
    } else {
        DrawCircleV(at(0.0f, 6.0f), 6.0f, {160, 225, 150, 255});   // snout
        DrawCircleV(at(-2.5f, 6.0f), 1.3f, OUTLINE);
        DrawCircleV(at(2.5f, 6.0f), 1.3f, OUTLINE);
    }
}

}  // namespace game
