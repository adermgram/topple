#include "game/block.h"
#include <cmath>

namespace game {

namespace {

Color Shade(Color c, float factor) {
    return {(unsigned char)(c.r * factor), (unsigned char)(c.g * factor),
            (unsigned char)(c.b * factor), c.a};
}

}  // namespace

Block::Block(physics::World& world, Vector2 centerPx, Vector2 sizePx,
             physics::BodyType type, Color color, const physics::Material& material)
    : body_(world.CreateBox(centerPx, sizePx, type, material)), size_(sizePx), color_(color) {}

void Block::Draw() const {
    Vector2 center = physics::ToPixels(body_->GetPosition());
    float angle = body_->GetAngle();   // radians; y-down makes positive = clockwise, same as raylib

    DrawRectanglePro({center.x, center.y, size_.x, size_.y}, {size_.x / 2, size_.y / 2},
                     angle * RAD2DEG, color_);

    // Outline from the rotated corners
    float c = std::cos(angle), s = std::sin(angle);
    Vector2 h = {size_.x / 2, size_.y / 2};
    const Vector2 local[4] = {{-h.x, -h.y}, {h.x, -h.y}, {h.x, h.y}, {-h.x, h.y}};
    Vector2 corner[4];
    for (int i = 0; i < 4; i++)
        corner[i] = {center.x + local[i].x * c - local[i].y * s,
                     center.y + local[i].x * s + local[i].y * c};
    Color outline = Shade(color_, 0.6f);
    for (int i = 0; i < 4; i++) DrawLineEx(corner[i], corner[(i + 1) % 4], 2.0f, outline);
}

}  // namespace game
