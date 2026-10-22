#pragma once

#include "game/ball.h"
#include "game/block.h"
#include "game/effects.h"
#include "game/level_def.h"
#include "game/target.h"
#include "physics/world.h"
#include "raylib.h"
#include <vector>

namespace game {

// What happened during one Update, for the owner to react to (sounds, rules, ...)
struct LevelEvents {
    int targetsDestroyed = 0;
    float strongestImpact = 0.0f;   // hardest collision this update; small values are just resting contact
};

// Everything that lives in the physics world for one puzzle: terrain, structures,
// targets and fired balls. Knows nothing about the rules (win/lose), sound or the UI.
class Level {
public:
    explicit Level(const LevelDef& def);

    LevelEvents Update(float dt);
    void Draw() const;

    void Fire(Vector2 from, Vector2 velocityPxPerSec);

    int shots() const { return shots_; }
    int targetsRemaining() const { return (int)targets_.size(); }
    bool IsSettled() const { return world_.AllAsleep(); }
    Vector2 slingAnchor() const { return layout::SLING_ANCHOR; }
    float groundY() const { return layout::GROUND_TOP; }

private:
    void Build(const LevelDef& def);
    void ApplyImpacts();
    int RemoveDestroyedTargets();   // returns how many were removed

    // Declared first: blocks, targets and balls hold pointers into the world
    physics::World world_;
    std::vector<Block> blocks_;
    std::vector<Target> targets_;
    std::vector<Ball> balls_;
    Effects effects_;
    int shots_;
};

}  // namespace game
