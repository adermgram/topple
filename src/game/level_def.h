#pragma once

#include "config.h"
#include "game/ball_kind.h"
#include "game/block_kind.h"
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

struct BlockSpec {
    BlockKind kind;
    Vector2 center;   // px
    Vector2 size;     // px
};

// A puzzle described as plain data. Level turns it into a physics world.
struct LevelDef {
    std::string name;
    std::vector<BallKind> ammo;   // the balls you get, in firing order
    std::vector<BlockSpec> blocks;
    std::vector<Vector2> targets;   // centres, px
};

// Every level, in play order. New levels go at the end: saved stars are stored by position.
const std::vector<LevelDef>& Levels();

}  // namespace game
