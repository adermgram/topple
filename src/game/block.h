#pragma once

#include "physics/world.h"
#include "raylib.h"

namespace game {

// A rectangular physics body that knows how to draw itself.
// The body belongs to the physics::World, which must outlive the Block.
class Block {
public:
    Block(physics::World& world, Vector2 centerPx, Vector2 sizePx,
          physics::BodyType type, Color color,
          const physics::Material& material = {});

    void Draw() const;

private:
    b2Body* body_;      // non-owning
    Vector2 size_;
    Color color_;
};

}  // namespace game
