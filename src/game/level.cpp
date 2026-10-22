#include "game/level.h"
#include "config.h"
#include <algorithm>

using physics::BodyType;

namespace game {

namespace {

constexpr float GROUND_WIDTH = 6000.0f;   // far wider than the screen so balls can't fall off the edge

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

    for (const BlockSpec& block : def.blocks)
        blocks_.emplace_back(world_, block.center, block.size, BodyType::Dynamic, ColorFor(block.kind));
    for (const Vector2& target : def.targets)
        targets_.emplace_back(world_, target);
}

void Level::Fire(Vector2 from, Vector2 velocityPxPerSec) {
    balls_.emplace_back(world_, from);
    balls_.back().Launch(velocityPxPerSec);
}

void Level::Update(float dt) {
    world_.Update(dt);
    ApplyImpacts();
    for (Target& target : targets_) target.Update(dt);
    RemoveDestroyedTargets();
    effects_.Update(dt);
}

void Level::ApplyImpacts() {
    for (const physics::Impact& impact : world_.impacts())
        for (Target& target : targets_)
            if (target.body() == impact.a || target.body() == impact.b)
                target.ApplyImpact(impact.impulse);
}

void Level::RemoveDestroyedTargets() {
    for (auto it = targets_.begin(); it != targets_.end();) {
        if (!it->destroyed()) {
            ++it;
            continue;
        }
        effects_.Burst(it->position(), Target::COLOR, 16);
        world_.DestroyBody(it->body());
        it = targets_.erase(it);
    }
}

void Level::Draw() const {
    for (const Block& block : blocks_) block.Draw();
    for (const Target& target : targets_) target.Draw();
    for (const Ball& ball : balls_) ball.Draw();
    effects_.Draw();
}

}  // namespace game
