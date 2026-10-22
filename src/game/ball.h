#pragma once

#include "physics/world.h"
#include "raylib.h"

namespace game {

// A launched projectile. The body belongs to the physics::World, which must outlive the Ball.
class Ball {
public:
    static constexpr float RADIUS = 16.0f;
    static constexpr Color COLOR{232, 90, 74, 255};

    Ball(physics::World& world, Vector2 centerPx);

    void Launch(Vector2 velocityPxPerSec);
    void Draw() const;

    // Also used by the slingshot to draw the ball waiting in the sling
    static void DrawAt(Vector2 centerPx, float angleRad);

private:
    b2Body* body_;   // non-owning
};

}  // namespace game
