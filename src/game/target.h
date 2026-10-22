#pragma once

#include "physics/world.h"
#include "raylib.h"

namespace game {

// Something to knock down. Hard hits wear its health down; it is destroyed at zero.
// It blinks when idle, goes dizzy when hit and looks sicker as it gets hurt.
// The body belongs to the physics::World, which must outlive the Target.
class Target {
public:
    static constexpr float RADIUS = 18.0f;
    static constexpr Color COLOR{120, 205, 120, 255};

    Target(physics::World& world, Vector2 centerPx);

    void ApplyImpact(float impulse);   // gentle contact is ignored
    void Update(float dt);
    void Draw() const;

    bool destroyed() const { return health_ <= 0.0f; }
    b2Body* body() const { return body_; }
    Vector2 position() const;

private:
    b2Body* body_;   // non-owning
    float health_;
    float flash_ = 0.0f;       // seconds of hit highlight left; also drives the squash
    float dazed_ = 0.0f;       // seconds of dizzy face left
    float time_ = 0.0f;
    float blinkOffset_;        // keeps the targets from blinking in unison
};

}  // namespace game
