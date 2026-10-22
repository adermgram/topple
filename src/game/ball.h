#pragma once

#include "game/ball_kind.h"
#include "physics/world.h"
#include "raylib.h"

namespace game {

// A launched projectile. What kind it is decides its size, weight and colour.
// The body belongs to the physics::World, which must outlive the Ball.
class Ball {
public:
    Ball(physics::World& world, Vector2 centerPx, BallKind kind);

    void Launch(Vector2 velocityPxPerSec);
    void Draw() const;

    BallKind kind() const { return kind_; }
    b2Body* body() const { return body_; }
    Vector2 position() const;
    Vector2 velocity() const;   // px/s

    static float RadiusOf(BallKind kind);
    static Color ColorOf(BallKind kind);
    static const char* NameOf(BallKind kind);

    // Also used by the slingshot to draw the ball waiting in the sling
    static void DrawAt(Vector2 centerPx, float angleRad, BallKind kind);

private:
    b2Body* body_;   // non-owning
    BallKind kind_;
};

}  // namespace game
