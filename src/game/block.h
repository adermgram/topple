#pragma once

#include "game/block_kind.h"
#include "physics/world.h"
#include "raylib.h"

namespace game {

    /* A rectangular physics body that knows how to draw itself. What it is made of
      (BlockKind) decides its weight, colour and whether it can break.
      The body belongs to the physics::World, which must outlive the Block. */
    class Block {
    public:
        Block(physics::World& world, Vector2 centerPx, Vector2 sizePx, physics::BodyType type, BlockKind kind);

        void ApplyImpact(float impulse);   // only fragile kinds react; gentle contact is ignored
        void Draw() const;

        bool breakable() const;
        bool destroyed() const { return health_ <= 0.0f; }
        BlockKind kind() const { return kind_; }
        b2Body* body() const { return body_; }
        Vector2 position() const;

    private:
        b2Body* body_;      // non-owning
        Vector2 size_;
        BlockKind kind_;
        float health_;
    };

}  // namespace game
