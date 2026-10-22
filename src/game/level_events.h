#pragma once

namespace game {

// What happened during one Level::Update, for the owner to react to
struct LevelEvents {
    int targetsDestroyed = 0;
    int glassShattered = 0;
    float strongestImpact = 0.0f;   // hardest collision; resting contact stays below ~6
};

}  // namespace game
