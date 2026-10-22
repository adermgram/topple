#pragma once

namespace game {

enum class BlockKind {
    Plank,     // light wood: pillars and roofs
    Crate,     // light wood: boxes
    Stone,     // heavy and unbreakable
    Glass,     // light and fragile: cracks, then shatters
    Terrain,   // the ground. Not used in level data
    Wall,      // invisible, keeps things on screen. Not used in level data
};

}  // namespace game
