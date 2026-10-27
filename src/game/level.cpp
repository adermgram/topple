#include "game/level.h"
#include "config.h"
#include <algorithm>
#include <cmath>

using physics::BodyType;

namespace game {

    namespace {

        constexpr float GROUND_WIDTH = 6000.0f;   // far wider than the screen
        constexpr float FOUNDATION_DEPTH = 200.0f;   // the ground continues below the screen, so a camera shake never shows its edge
        constexpr float WALL_THICKNESS = 200.0f;     // thick enough that fast objects can't tunnel through
        constexpr float WALL_HEIGHT = 4000.0f;

        constexpr float SETTLE_GRACE_SECONDS = 0.6f;

        constexpr float MIN_SPLIT_SPEED = 60.0f;       // px/s; a slower ball is just lying there
        constexpr float SPLIT_SPREAD_RADIANS = 0.24f;
        constexpr float FRAGMENT_SPACING = 26.0f;      // more than a fragment's diameter, so they don't start overlapping
        constexpr int SPLIT_PARTICLES = 10;

        constexpr int TARGET_PARTICLES = 20;
        constexpr float TARGET_RING_RADIUS = 48.0f;
        constexpr int GLASS_SHARDS = 18;

        // Same scale as the hit sound: settling stays under ~6
        constexpr float DUST_MIN_IMPACT = 9.0f;
        constexpr int DUST_MAX_PER_UPDATE = 3;         // the hardest few, so a pile-up doesn't fog the screen
        constexpr float DUST_IMPULSE_PER_PUFF = 5.0f;
        constexpr int DUST_MIN_PUFFS = 3;
        constexpr int DUST_MAX_PUFFS = 12;

        const Color GLASS_SHARD_COLOR{190, 235, 250, 255};
        const Color STONE_DUST{176, 180, 192, 255};
        const Color WOOD_DUST{214, 176, 128, 255};
        const Color GROUND_DUST{140, 150, 178, 255};

        // Erases the destroyed items, calling onRemove for each first. Returns how many there were.
        template <typename Item, typename OnRemove>
        int RemoveDestroyed(std::vector<Item>& items, OnRemove onRemove) {
            int removed = 0;
            for (auto it = items.begin(); it != items.end();) {
                if (!it->destroyed()) {
                    ++it;
                    continue;
                }
                onRemove(*it);
                it = items.erase(it);
                removed++;
            }
            return removed;
        }

    }  // namespace

    Level::Level(const LevelDef& def) : ammo_(def.ammo) { Build(def); }

    void Level::Build(const LevelDef& def) {
        using layout::GROUND_HEIGHT;
        using layout::GROUND_TOP;

        const float middle = config::SCREEN_W / 2.0f;
        blocks_.emplace_back(world_, Vector2{middle, GROUND_TOP + GROUND_HEIGHT / 2},
                            Vector2{GROUND_WIDTH, GROUND_HEIGHT}, BodyType::Static, BlockKind::Terrain);
        blocks_.emplace_back(world_, Vector2{middle, GROUND_TOP + GROUND_HEIGHT + FOUNDATION_DEPTH / 2},
                            Vector2{GROUND_WIDTH, FOUNDATION_DEPTH}, BodyType::Static, BlockKind::Terrain);

        /* Walls just outside the screen. Without them a hit target can roll away for good,
        leaving a level that can no longer be won */
        const float wallY = GROUND_TOP - WALL_HEIGHT / 2;
        for (float side : {-1.0f, 1.0f}) {
            const float edge = side < 0 ? 0.0f : static_cast<float>(config::SCREEN_W);
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

    const Ball* Level::FindSplittable() const {
        for (const Ball& ball : balls_) {
            if (ball.kind() != BallKind::Splitter) continue;
            const Vector2 v = ball.velocity();
            if (v.x * v.x + v.y * v.y >= MIN_SPLIT_SPEED * MIN_SPLIT_SPEED) return &ball;
        }
        return nullptr;
    }

    bool Level::canTriggerAbility() const { return FindSplittable() != nullptr; }

    bool Level::TriggerAbility() {
        const Ball* ball = FindSplittable();
        if (!ball) return false;

        const Vector2 position = ball->position();
        const Vector2 velocity = ball->velocity();
        const float speed = std::hypot(velocity.x, velocity.y);
        const float heading = std::atan2(velocity.y, velocity.x);

        world_.DestroyBody(ball->body());
        balls_.erase(balls_.begin() + (ball - balls_.data()));

        const Vector2 sideways = {-std::sin(heading), std::cos(heading)};
        for (int i = -1; i <= 1; i++) {
            const float offset = static_cast<float>(i);
            const float angle = heading + offset * SPLIT_SPREAD_RADIANS;
            const Vector2 start = {position.x + sideways.x * offset * FRAGMENT_SPACING,
                                position.y + sideways.y * offset * FRAGMENT_SPACING};
            Fire(start, {std::cos(angle) * speed, std::sin(angle) * speed}, BallKind::Fragment);
        }
        effects_.Burst(position, Ball::ColorOf(BallKind::Splitter), SPLIT_PARTICLES);
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

    // Dust where the hardest collisions happened, coloured by what was hit. Glass gets none, it shatters instead.
    void Level::EmitImpactDust() {
        std::vector<const physics::Impact*> hard;
        for (const physics::Impact& impact : world_.impacts())
            if (impact.impulse >= DUST_MIN_IMPACT) hard.push_back(&impact);

        const std::size_t keep = std::min<std::size_t>(hard.size(), DUST_MAX_PER_UPDATE);
        std::partial_sort(hard.begin(), hard.begin() + static_cast<std::ptrdiff_t>(keep), hard.end(),
                        [](const physics::Impact* a, const physics::Impact* b) { return a->impulse > b->impulse; });
        hard.resize(keep);

        for (const physics::Impact* impact : hard) {
            bool stone = false, wood = false, glass = false;
            for (const Block& block : blocks_) {
                if (block.body() != impact->a && block.body() != impact->b) continue;
                stone |= block.kind() == BlockKind::Stone;
                glass |= block.kind() == BlockKind::Glass;
                wood |= block.kind() == BlockKind::Plank || block.kind() == BlockKind::Crate;
            }
            if (glass) continue;

            const Color color = stone ? STONE_DUST : wood ? WOOD_DUST : GROUND_DUST;
            const int puffs = std::clamp(static_cast<int>(impact->impulse / DUST_IMPULSE_PER_PUFF), DUST_MIN_PUFFS, DUST_MAX_PUFFS);
            effects_.Dust(impact->point, color, puffs);
        }
    }

    int Level::RemoveDestroyedTargets() {
        return RemoveDestroyed(targets_, [this](Target& target) {
            effects_.Burst(target.position(), Target::COLOR, TARGET_PARTICLES);
            effects_.Ring(target.position(), Target::COLOR, TARGET_RING_RADIUS);
            world_.DestroyBody(target.body());
        });
    }

    int Level::RemoveDestroyedBlocks() {
        return RemoveDestroyed(blocks_, [this](Block& block) {
            effects_.Burst(block.position(), GLASS_SHARD_COLOR, GLASS_SHARDS);
            world_.DestroyBody(block.body());
        });
    }

    void Level::Draw() const {
        for (const Block& block : blocks_) block.Draw();
        for (const Target& target : targets_) target.Draw();
        for (const Ball& ball : balls_) ball.Draw();
        effects_.Draw();
    }

}  // namespace game
