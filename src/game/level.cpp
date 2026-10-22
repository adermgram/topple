#include "game/level.h"
#include "config.h"
#include <algorithm>

using physics::BodyType;

namespace game {

namespace {

constexpr float GROUND_WIDTH = 6000.0f;   // far wider than the screen so nothing can fall off the edge
constexpr float WALL_THICKNESS = 200.0f;  // thick, so fast objects can't tunnel through
constexpr float WALL_HEIGHT = 4000.0f;

const Color GROUND_COLOR{58, 68, 98, 255};
const Color PLANK_COLOR{170, 118, 70, 255};
const Color CRATE_COLOR{196, 138, 82, 255};

Color ColorFor(BlockKind kind) { return kind == BlockKind::Plank ? PLANK_COLOR : CRATE_COLOR; }

}  // namespace

Level::Level(const LevelDef& def) : shots_(def.shots) { Build(def); }

void Level::Build(const LevelDef& def) {
    using layout::GROUND_HEIGHT;
    using layout::GROUND_TOP;

    blocks_.emplace_back(world_, Vector2{config::SCREEN_W / 2.0f, GROUND_TOP + GROUND_HEIGHT / 2},
                         Vector2{GROUND_WIDTH, GROUND_HEIGHT}, BodyType::Static, GROUND_COLOR);

    // Invisible walls just outside the screen keep targets (and balls) in the playfield:
    // a hit target that rolls away could otherwise leave a level that can never be won
    const float wallY = GROUND_TOP - WALL_HEIGHT / 2;
    for (float edge : {0.0f, (float)config::SCREEN_W}) {
        float side = (edge == 0.0f) ? -1.0f : 1.0f;
        blocks_.emplace_back(world_, Vector2{edge + side * WALL_THICKNESS / 2, wallY},
                             Vector2{WALL_THICKNESS, WALL_HEIGHT}, BodyType::Static, GROUND_COLOR);
    }

    for (const BlockSpec& block : def.blocks)
        blocks_.emplace_back(world_, block.center, block.size, BodyType::Dynamic, ColorFor(block.kind));
    for (const Vector2& target : def.targets)
        targets_.emplace_back(world_, target);
}

void Level::Fire(Vector2 from, Vector2 velocityPxPerSec) {
    balls_.emplace_back(world_, from);
    balls_.back().Launch(velocityPxPerSec);
}

LevelEvents Level::Update(float dt) {
    LevelEvents events;

    world_.Update(dt);
    for (const physics::Impact& impact : world_.impacts())
        events.strongestImpact = std::max(events.strongestImpact, impact.impulse);
    ApplyImpacts();

    for (Target& target : targets_) target.Update(dt);
    events.targetsDestroyed = RemoveDestroyedTargets();
    effects_.Update(dt);
    return events;
}

void Level::ApplyImpacts() {
    for (const physics::Impact& impact : world_.impacts())
        for (Target& target : targets_)
            if (target.body() == impact.a || target.body() == impact.b)
                target.ApplyImpact(impact.impulse);
}

int Level::RemoveDestroyedTargets() {
    int removed = 0;
    for (auto it = targets_.begin(); it != targets_.end();) {
        if (!it->destroyed()) {
            ++it;
            continue;
        }
        effects_.Burst(it->position(), Target::COLOR, 16);
        world_.DestroyBody(it->body());
        it = targets_.erase(it);
        removed++;
    }
    return removed;
}

void Level::Draw() const {
    for (const Block& block : blocks_) block.Draw();
    for (const Target& target : targets_) target.Draw();
    for (const Ball& ball : balls_) ball.Draw();
    effects_.Draw();
}

}  // namespace game
