#include "game/ball.h"
#include <cmath>

namespace game {

namespace {

// Heavier than a crate so it knocks things over
const physics::Material BALL_MATERIAL{3.0f, 0.5f, 0.2f};

}  // namespace

Ball::Ball(physics::World& world, Vector2 centerPx)
    : body_(world.CreateCircle(centerPx, RADIUS, physics::BodyType::Dynamic, BALL_MATERIAL)) {}

void Ball::Launch(Vector2 velocityPxPerSec) {
    body_->SetLinearVelocity(physics::ToMeters(velocityPxPerSec));
}

void Ball::Draw() const {
    DrawAt(physics::ToPixels(body_->GetPosition()), body_->GetAngle());
}

void Ball::DrawAt(Vector2 centerPx, float angleRad) {
    DrawCircleV(centerPx, RADIUS, COLOR);
    // A tick mark makes the spin visible
    Vector2 tick = {centerPx.x + std::cos(angleRad) * RADIUS * 0.6f,
                    centerPx.y + std::sin(angleRad) * RADIUS * 0.6f};
    DrawLineEx(centerPx, tick, 3.0f, {120, 30, 24, 255});
    DrawCircleLinesV(centerPx, RADIUS, {140, 40, 30, 255});
}

}  // namespace game
