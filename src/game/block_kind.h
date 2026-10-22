#pragma once

namespace game {

enum class BlockKind {
    Plank,     // light wood: pillars and roofs
    Crate,     // light wood: boxes
    Stone,     // heavy and unbreakable
    Glass,     // light and fragile: cracks, then shatters
    Terrain,   // ground and walls, never moves (not used in level data)
};

}  // namespace game
