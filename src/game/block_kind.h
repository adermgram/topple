#pragma once

namespace game {

enum class BlockKind {
    Plank,     // light wood: pillars and roofs
    Crate,     // light wood: boxes
    Stone,     // heavy and unbreakable
    Glass,     // light and fragile: cracks, then shatters
    Terrain,   // the visible ground: never moves (not used in level data)
    Wall,      // an invisible boundary at the screen edge: never moves (not used in level data)
};

}  // namespace game
