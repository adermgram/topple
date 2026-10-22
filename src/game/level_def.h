#pragma once

#include "config.h"
#include "raylib.h"
#include <string>
#include <vector>

namespace game {

// Fixed parts of the scene shared by every level
namespace layout {
constexpr float GROUND_HEIGHT = 80.0f;
constexpr float GROUND_TOP = config::SCREEN_H - GROUND_HEIGHT;
constexpr Vector2 SLING_ANCHOR{230.0f, GROUND_TOP - 190.0f};
}  // namespace layout

enum class BlockKind { Plank, Crate };

struct BlockSpec {
    BlockKind kind;
    Vector2 center;   // px
    Vector2 size;     // px
};

// A puzzle described as plain data. Level turns it into a physics world.
struct LevelDef {
    std::string name;
    int shots;
    std::vector<BlockSpec> blocks;
    std::vector<Vector2> targets;   // centres, px
};

// Every level, in play order
const std::vector<LevelDef>& Levels();

}  // namespace game
