#include "game/target.h"
#include <cmath>

namespace game {

namespace {

constexpr float MAX_HEALTH = 15.0f;
constexpr float MIN_DAMAGING_IMPULSE = 4.0f;   // below this (resting, light bumps) nothing happens
constexpr float FLASH_TIME = 0.15f;

const physics::Material TARGET_MATERIAL{1.0f, 0.6f, 0.1f};

Vector2 Rotate(Vector2 v, float angle) {
    float c = std::cos(angle), s = std::sin(angle);
    return {v.x * c - v.y * s, v.x * s + v.y * c};
}

}  // namespace

Target::Target(physics::World& world, Vector2 centerPx)
    : body_(world.CreateCircle(centerPx, RADIUS, physics::BodyType::Dynamic, TARGET_MATERIAL)),
      health_(MAX_HEALTH) {}

Vector2 Target::position() const { return physics::ToPixels(body_->GetPosition()); }

void Target::ApplyImpact(float impulse) {
    if (impulse < MIN_DAMAGING_IMPULSE) return;
    health_ -= impulse - MIN_DAMAGING_IMPULSE;
    flash_ = FLASH_TIME;
}

void Target::Update(float dt) {
    if (flash_ > 0.0f) flash_ -= dt;
}

void Target::Draw() const {
    Vector2 c = position();
    float angle = body_->GetAngle();

    Color fill = flash_ > 0.0f ? Color{235, 250, 235, 255} : COLOR;
    DrawCircleV(c, RADIUS, fill);
    DrawCircleLinesV(c, RADIUS, {50, 110, 60, 255});

    // Face, rotating with the body
    auto at = [&](float x, float y) {
        Vector2 o = Rotate({x, y}, angle);
        return Vector2{c.x + o.x, c.y + o.y};
    };
    DrawCircleV(at(-6.5f, -4.0f), 4.5f, WHITE);
    DrawCircleV(at(6.5f, -4.0f), 4.5f, WHITE);
    DrawCircleV(at(-6.0f, -4.0f), 2.2f, {30, 40, 30, 255});
    DrawCircleV(at(6.0f, -4.0f), 2.2f, {30, 40, 30, 255});
    DrawCircleV(at(0.0f, 6.0f), 6.0f, {160, 225, 150, 255});
    DrawCircleV(at(-2.5f, 6.0f), 1.3f, {50, 110, 60, 255});
    DrawCircleV(at(2.5f, 6.0f), 1.3f, {50, 110, 60, 255});
}

}  // namespace game
