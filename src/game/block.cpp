#include "game/block.h"
#include "raymath.h"
#include "util/color.h"
#include <cmath>
#include <limits>

namespace game {

namespace {

constexpr float UNBREAKABLE = std::numeric_limits<float>::infinity();

struct Style {
    physics::Material material;
    Color color;
    float health;
    float minDamagingImpulse;
};

const Style& StyleFor(BlockKind kind) {
    //                          density friction bounce
    static const Style plank   {{1.0f, 0.6f, 0.05f}, {170, 118, 70, 255},  UNBREAKABLE, 0.0f};
    static const Style crate   {{1.0f, 0.6f, 0.05f}, {196, 138, 82, 255},  UNBREAKABLE, 0.0f};
    static const Style stone   {{2.5f, 0.7f, 0.02f}, {128, 134, 148, 255}, UNBREAKABLE, 0.0f};
    static const Style glass   {{0.6f, 0.3f, 0.05f}, {150, 215, 235, 150}, 10.0f,       6.0f};
    static const Style terrain {{1.0f, 0.6f, 0.05f}, {58, 68, 98, 255},    UNBREAKABLE, 0.0f};
    static const Style wall    {{1.0f, 0.6f, 0.05f}, {0, 0, 0, 0},         UNBREAKABLE, 0.0f};

    switch (kind) {
    case BlockKind::Plank:   return plank;
    case BlockKind::Crate:   return crate;
    case BlockKind::Stone:   return stone;
    case BlockKind::Glass:   return glass;
    case BlockKind::Terrain: return terrain;
    case BlockKind::Wall:    return wall;
    }
    return plank;
}

}  // namespace

Block::Block(physics::World& world, Vector2 centerPx, Vector2 sizePx, physics::BodyType type, BlockKind kind)
    : body_(world.CreateBox(centerPx, sizePx, type, StyleFor(kind).material)),
      size_(sizePx),
      kind_(kind),
      health_(StyleFor(kind).health) {}

bool Block::breakable() const { return std::isfinite(StyleFor(kind_).health); }

Vector2 Block::position() const { return physics::ToPixels(body_->GetPosition()); }

void Block::ApplyImpact(float impulse) {
    const Style& style = StyleFor(kind_);
    if (!breakable() || impulse < style.minDamagingImpulse) return;
    health_ -= impulse - style.minDamagingImpulse;
}

void Block::Draw() const {
    if (kind_ == BlockKind::Wall) return;

    const Style& style = StyleFor(kind_);
    const Vector2 center = position();
    const float angle = body_->GetAngle();   // y points down, so a positive angle turns clockwise, like raylib's

    // Takes a point relative to the block's centre, before rotation
    auto line = [&](Vector2 from, Vector2 to, float thickness, Color color) {
        DrawLineEx(Vector2Add(center, Vector2Rotate(from, angle)),
                   Vector2Add(center, Vector2Rotate(to, angle)), thickness, color);
    };

    DrawRectanglePro({center.x, center.y, size_.x, size_.y}, {size_.x / 2, size_.y / 2},
                     angle * RAD2DEG, style.color);

    const Vector2 h = {size_.x / 2, size_.y / 2};
    const Color outline = (kind_ == BlockKind::Glass) ? Color{225, 245, 255, 235} : util::Shade(style.color, 0.6f);
    const Vector2 corner[4] = {{-h.x, -h.y}, {h.x, -h.y}, {h.x, h.y}, {-h.x, h.y}};
    for (int i = 0; i < 4; i++) line(corner[i], corner[(i + 1) % 4], 2.0f, outline);

    if (kind_ == BlockKind::Stone) {
        const Color light = util::Shade(style.color, 1.25f);
        line({-h.x + 3, h.y - 3}, {-h.x + 3, -h.y + 3}, 2.0f, light);
        line({-h.x + 3, -h.y + 3}, {h.x - 3, -h.y + 3}, 2.0f, light);
    } else if (kind_ == BlockKind::Glass) {
        const Color glint{255, 255, 255, 200};
        if (size_.x >= size_.y) line({-0.35f * size_.x, -0.25f * size_.y}, {-0.15f * size_.x, -0.25f * size_.y}, 2.0f, glint);
        else line({-0.25f * size_.x, -0.35f * size_.y}, {-0.25f * size_.x, -0.15f * size_.y}, 2.0f, glint);

        if (health_ < style.health) {
            const Color crack{255, 255, 255, 220};
            line({0, 0}, {0.3f * size_.x, -0.4f * size_.y}, 1.5f, crack);
            line({0, 0}, {-0.35f * size_.x, 0.3f * size_.y}, 1.5f, crack);
            line({0, 0}, {0.15f * size_.x, 0.45f * size_.y}, 1.5f, crack);
        }
    }
}

}  // namespace game
