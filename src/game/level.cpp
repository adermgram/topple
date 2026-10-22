#include "game/level.h"
#include "config.h"
#include <algorithm>

using physics::BodyType;

namespace game {

namespace {

constexpr float GROUND_HEIGHT = 80.0f;
constexpr float GROUND_TOP = config::SCREEN_H - GROUND_HEIGHT;
constexpr float GROUND_WIDTH = 6000.0f;   // far wider than the screen so balls can't fall off the edge

constexpr float PILLAR_W = 18.0f;
constexpr float PLANK_T = 16.0f;
constexpr float CRATE_SIZE = 36.0f;
constexpr float GAP = 4.0f;               // structures start slightly apart and settle into place

const Vector2 SLING_ANCHOR{230.0f, GROUND_TOP - 190.0f};
const Color GROUND_COLOR{58, 68, 98, 255};
const Color PLANK_COLOR{170, 118, 70, 255};
const Color CRATE_COLOR{196, 138, 82, 255};

}  // namespace

Level::Level() { Build(); }

Vector2 Level::slingAnchor() const { return SLING_ANCHOR; }
float Level::groundY() const { return GROUND_TOP; }

// Two small towers, each a pair of pillars with a roof, with targets inside and on top
void Level::Build() {
    blocks_.emplace_back(world_, Vector2{config::SCREEN_W / 2.0f, GROUND_TOP + GROUND_HEIGHT / 2},
                         Vector2{GROUND_WIDTH, GROUND_HEIGHT}, BodyType::Static, GROUND_COLOR);

    // Returns the y of the roof's top surface
    auto tower = [&](float centerX, float pillarHeight, float spread) {
        float pillarY = GROUND_TOP - pillarHeight / 2 - GAP;
        for (float side : {-1.0f, 1.0f})
            blocks_.emplace_back(world_, Vector2{centerX + side * spread / 2, pillarY},
                                 Vector2{PILLAR_W, pillarHeight}, BodyType::Dynamic, PLANK_COLOR);

        float roofY = GROUND_TOP - pillarHeight - GAP - PLANK_T / 2 - GAP;
        blocks_.emplace_back(world_, Vector2{centerX, roofY},
                             Vector2{spread + PILLAR_W * 2, PLANK_T}, BodyType::Dynamic, PLANK_COLOR);
        return roofY - PLANK_T / 2;
    };

    auto targetOnGround = [&](float x) {
        targets_.emplace_back(world_, Vector2{x, GROUND_TOP - Target::RADIUS - 2});
    };

    // Tower A: a target inside and a crate on the roof
    float roofA = tower(880.0f, 80.0f, 80.0f);
    targetOnGround(880.0f);
    blocks_.emplace_back(world_, Vector2{880.0f, roofA - CRATE_SIZE / 2 - GAP},
                         Vector2{CRATE_SIZE, CRATE_SIZE}, BodyType::Dynamic, CRATE_COLOR);

    // Tower B: taller, with a target inside and another on the roof
    float roofB = tower(1060.0f, 110.0f, 80.0f);
    targetOnGround(1060.0f);
    targets_.emplace_back(world_, Vector2{1060.0f, roofB - Target::RADIUS - GAP});
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
