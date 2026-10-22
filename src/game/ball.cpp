#include "game/ball.h"
#include "util/color.h"
#include <cmath>

namespace game {

namespace {

struct Style {
    float radius;
    physics::Material material;
    Color color;
    const char* name;
};

const Style& StyleFor(BallKind kind) {
    //                          radius  density friction bounce
    static const Style standard {16.0f, {3.0f, 0.5f, 0.20f}, {232, 90, 74, 255},   "Ball"};
    static const Style heavy    {18.0f, {5.5f, 0.6f, 0.05f}, {84, 92, 112, 255},   "Heavy ball"};
    static const Style splitter {15.0f, {3.4f, 0.5f, 0.20f}, {250, 206, 70, 255},  "Splitter"};
    static const Style fragment {12.0f, {5.5f, 0.5f, 0.20f}, {255, 226, 120, 255}, "Fragment"};

    switch (kind) {
    case BallKind::Standard: return standard;
    case BallKind::Heavy:    return heavy;
    case BallKind::Splitter: return splitter;
    case BallKind::Fragment: return fragment;
    }
    return standard;
}

}  // namespace

float Ball::RadiusOf(BallKind kind) { return StyleFor(kind).radius; }
Color Ball::ColorOf(BallKind kind) { return StyleFor(kind).color; }
const char* Ball::NameOf(BallKind kind) { return StyleFor(kind).name; }

Ball::Ball(physics::World& world, Vector2 centerPx, BallKind kind)
    : body_(world.CreateCircle(centerPx, StyleFor(kind).radius, physics::BodyType::Dynamic,
                               StyleFor(kind).material)),
      kind_(kind) {}

Vector2 Ball::position() const { return physics::ToPixels(body_->GetPosition()); }
Vector2 Ball::velocity() const { return physics::ToPixels(body_->GetLinearVelocity()); }

void Ball::Launch(Vector2 velocityPxPerSec) {
    body_->SetLinearVelocity(physics::ToMeters(velocityPxPerSec));
}

void Ball::Draw() const { DrawAt(position(), body_->GetAngle(), kind_); }

void Ball::DrawAt(Vector2 centerPx, float angleRad, BallKind kind) {
    const Style& style = StyleFor(kind);
    const Color dark = util::Shade(style.color, 0.55f);

    DrawCircleV(centerPx, style.radius, style.color);
    if (kind == BallKind::Heavy) DrawCircleLinesV(centerPx, style.radius * 0.62f, util::Shade(style.color, 1.4f));

    Vector2 tick = {centerPx.x + std::cos(angleRad) * style.radius * 0.6f,
                    centerPx.y + std::sin(angleRad) * style.radius * 0.6f};
    DrawLineEx(centerPx, tick, kind == BallKind::Fragment ? 2.0f : 3.0f, dark);
    DrawCircleLinesV(centerPx, style.radius, dark);
}

}  // namespace game
