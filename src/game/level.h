#pragma once

#include "game/ball.h"
#include "game/block.h"
#include "game/effects.h"
#include "game/level_def.h"
#include "game/level_events.h"
#include "game/target.h"
#include "physics/world.h"
#include "raylib.h"
#include <vector>

namespace game {

    /* Everything that lives in the physics world for one puzzle: terrain, structures, targets and fired balls.
    It knows nothing about the rules, sound or the UI. */
    class Level {
    public:
        explicit Level(const LevelDef& def);

        LevelEvents Update(float dt);
        void Draw() const;

        void Fire(Vector2 from, Vector2 velocityPxPerSec, BallKind kind);

        // Splits a moving Splitter into three pieces; false if there is none to split
        bool TriggerAbility();
        bool canTriggerAbility() const;

        int shots() const { return static_cast<int>(ammo_.size()); }
        BallKind ammo(int index) const { return ammo_.at(static_cast<std::size_t>(index)); }
        int targetsRemaining() const { return static_cast<int>(targets_.size()); }
        bool isSettled() const { return world_.allAsleep(); }
        Vector2 slingAnchor() const { return layout::SLING_ANCHOR; }
        float groundY() const { return layout::GROUND_TOP; }

    private:
        void Build(const LevelDef& def);
        void ApplyImpacts();
        void EmitImpactDust();
        int RemoveDestroyedTargets();
        int RemoveDestroyedBlocks();
        const Ball* FindSplittable() const;

        // The world comes first: blocks, targets and balls hold pointers into it
        physics::World world_;
        std::vector<Block> blocks_;
        std::vector<Target> targets_;
        std::vector<Ball> balls_;
        Effects effects_;
        std::vector<BallKind> ammo_;
        float age_ = 0.0f;
    };

}  // namespace game
