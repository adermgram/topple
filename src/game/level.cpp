#include "game/level.h"
#include "config.h"
#include <algorithm>
#include <cmath>

using physics::BodyType;

namespace game {

namespace {

constexpr float GROUND_WIDTH = 6000.0f;   // far wider than the screen so nothing can fall off the edge
constexpr float WALL_THICKNESS = 200.0f;  // thick, so fast objects can't tunnel through
constexpr float WALL_HEIGHT = 4000.0f;

const Color GLASS_SHARD_COLOR{190, 235, 250, 255};

// Structures start a few pixels above what they rest on and drop into place. For this long
// after loading, the landing counts as nothing: no damage, no sounds.
constexpr float SETTLE_GRACE_SECONDS = 0.6f;

constexpr float MIN_SPLIT_SPEED = 60.0f;        // px/s
constexpr float SPLIT_SPREAD_RADIANS = 0.2f;    // about 11.5 degrees either side
constexpr float FRAGMENT_SPACING = 20.0f;       // px, more than a fragment's diameter

// Collisions below this raise no dust (same scale as the hit sound: settling stays under ~6)
constexpr float DUST_MIN_IMPACT = 9.0f;
constexpr int DUST_MAX_PER_UPDATE = 3;          // the hardest few; a pile-up shouldn't fog the screen
constexpr float DUST_IMPULSE_PER_PUFF = 5.0f;
constexpr int DUST_MIN_PUFFS = 3;
constexpr int DUST_MAX_PUFFS = 12;

constexpr float FOUNDATION_DEPTH = 200.0f;      // ground continues below the screen, for the shake

const Color STONE_DUST{176, 180, 192, 255};
const Color WOOD_DUST{214, 176, 128, 255};
const Color GROUND_DUST{140, 150, 178, 255};

}  // namespace

Level::Level(const LevelDef& def) : ammo_(def.ammo) { Build(def); }

void Level::Build(const LevelDef& def) {
    using layout::GROUND_HEIGHT;
    using layout::GROUND_TOP;

    blocks_.emplace_back(world_, Vector2{config::SCREEN_W / 2.0f, GROUND_TOP + GROUND_HEIGHT / 2},
                         Vector2{GROUND_WIDTH, GROUND_HEIGHT}, BodyType::Static, BlockKind::Terrain);
    blocks_.emplace_back(world_, Vector2{config::SCREEN_W / 2.0f, GROUND_TOP + GROUND_HEIGHT + FOUNDATION_DEPTH / 2},
                         Vector2{GROUND_WIDTH, FOUNDATION_DEPTH}, BodyType::Static, BlockKind::Terrain);

    // Invisible walls just outside the screen keep targets (and balls) in the playfield:
    // a hit target that rolls away could otherwise leave a level that can never be won
    const float wallY = GROUND_TOP - WALL_HEIGHT / 2;
    for (float edge : {0.0f, (float)config::SCREEN_W}) {
        float side = (edge == 0.0f) ? -1.0f : 1.0f;
        blocks_.emplace_back(world_, Vector2{edge + side * WALL_THICKNESS / 2, wallY},
                             Vector2{WALL_THICKNESS, WALL_HEIGHT}, BodyType::Static, BlockKind::Wall);
    }

    for (const BlockSpec& block : def.blocks)
        blocks_.emplace_back(world_, block.center, block.size, BodyType::Dynamic, block.kind);
    for (const Vector2& target : def.targets)
        targets_.emplace_back(world_, target);
}

void Level::Fire(Vector2 from, Vector2 velocityPxPerSec, BallKind kind) {
    balls_.emplace_back(world_, from, kind);
    balls_.back().Launch(velocityPxPerSec);
}

// A Splitter that has slowed to a crawl is just lying there: nothing left to split
const Ball* Level::FindSplittable() const {
    for (const Ball& ball : balls_) {
        if (ball.kind() != BallKind::Splitter) continue;
        Vector2 v = ball.velocity();
        if (v.x * v.x + v.y * v.y >= MIN_SPLIT_SPEED * MIN_SPLIT_SPEED) return &ball;
    }
    return nullptr;
}

bool Level::CanTriggerAbility() const { return FindSplittable() != nullptr; }

bool Level::TriggerAbility() {
    const Ball* ball = FindSplittable();
    if (!ball) return false;

    const Vector2 position = ball->position();
    const Vector2 velocity = ball->velocity();
    const float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

    world_.DestroyBody(ball->body());
    balls_.erase(balls_.begin() + (ball - balls_.data()));

    // Three pieces fan out around the old direction, side by side so they don't start overlapping
    const float heading = std::atan2(velocity.y, velocity.x);
    const Vector2 sideways = {-std::sin(heading), std::cos(heading)};
    for (int i = -1; i <= 1; i++) {
        float angle = heading + i * SPLIT_SPREAD_RADIANS;
        Vector2 start = {position.x + sideways.x * i * FRAGMENT_SPACING,
                         position.y + sideways.y * i * FRAGMENT_SPACING};
        Fire(start, {std::cos(angle) * speed, std::sin(angle) * speed}, BallKind::Fragment);
    }
    effects_.Burst(position, Ball::ColorOf(BallKind::Splitter), 10);
    return true;
}

LevelEvents Level::Update(float dt) {
    LevelEvents events;

    world_.Update(dt);
    age_ += dt;

    if (age_ >= SETTLE_GRACE_SECONDS) {
        for (const physics::Impact& impact : world_.impacts())
            events.strongestImpact = std::max(events.strongestImpact, impact.impulse);
        ApplyImpacts();
        EmitImpactDust();
    }

    for (Target& target : targets_) target.Update(dt);
    events.targetsDestroyed = RemoveDestroyedTargets();
    events.glassShattered = RemoveDestroyedBlocks();
    effects_.Update(dt);
    return events;
}

void Level::ApplyImpacts() {
    for (const physics::Impact& impact : world_.impacts()) {
        for (Target& target : targets_)
            if (target.body() == impact.a || target.body() == impact.b)
                target.ApplyImpact(impact.impulse);
        for (Block& block : blocks_)
            if (block.breakable() && (block.body() == impact.a || block.body() == impact.b))
                block.ApplyImpact(impact.impulse);
    }
}

// Dust at the point of the hardest collisions, coloured by what was hit. Glass gets none:
// it already shatters into shards.
void Level::EmitImpactDust() {
    std::vector<const physics::Impact*> hard;
    for (const physics::Impact& impact : world_.impacts())
        if (impact.impulse >= DUST_MIN_IMPACT) hard.push_back(&impact);
    std::sort(hard.begin(), hard.end(),
              [](const physics::Impact* a, const physics::Impact* b) { return a->impulse > b->impulse; });
    if ((int)hard.size() > DUST_MAX_PER_UPDATE) hard.resize(DUST_MAX_PER_UPDATE);

    for (const physics::Impact* impact : hard) {
        bool stone = false, wood = false, glass = false;
        for (const Block& block : blocks_) {
            if (block.body() != impact->a && block.body() != impact->b) continue;
            stone |= block.kind() == BlockKind::Stone;
            glass |= block.kind() == BlockKind::Glass;
            wood |= block.kind() == BlockKind::Plank || block.kind() == BlockKind::Crate;
        }
        if (glass) continue;

        Color color = stone ? STONE_DUST : wood ? WOOD_DUST : GROUND_DUST;
        int puffs = std::clamp((int)(impact->impulse / DUST_IMPULSE_PER_PUFF), DUST_MIN_PUFFS, DUST_MAX_PUFFS);
        effects_.Dust(impact->point, color, puffs);
    }
}

int Level::RemoveDestroyedTargets() {
    int removed = 0;
    for (auto it = targets_.begin(); it != targets_.end();) {
        if (!it->destroyed()) {
            ++it;
            continue;
        }
        effects_.Burst(it->position(), Target::COLOR, 20);
        effects_.Ring(it->position(), Target::COLOR, 48.0f);
        world_.DestroyBody(it->body());
        it = targets_.erase(it);
        removed++;
    }
    return removed;
}

int Level::RemoveDestroyedBlocks() {
    int removed = 0;
    for (auto it = blocks_.begin(); it != blocks_.end();) {
        if (!it->destroyed()) {
            ++it;
            continue;
        }
        effects_.Burst(it->position(), GLASS_SHARD_COLOR, 18);
        world_.DestroyBody(it->body());
        it = blocks_.erase(it);
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
