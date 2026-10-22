#include "game/screen_shake.h"
#include <algorithm>
#include <cmath>

namespace game {

namespace {

constexpr float MAX_OFFSET = 14.0f;     // px at full trauma
constexpr float MAX_ROTATION = 1.2f;    // degrees at full trauma
constexpr float DECAY_PER_SECOND = 1.6f;

}  // namespace

void ScreenShake::AddTrauma(float amount) { trauma_ = std::min(1.0f, trauma_ + amount); }

void ScreenShake::Raise(float level) { trauma_ = std::min(1.0f, std::max(trauma_, level)); }

void ScreenShake::Update(float dt) {
    time_ += dt;
    trauma_ = std::max(0.0f, trauma_ - DECAY_PER_SECOND * dt);
}

void ScreenShake::Reset() { trauma_ = 0.0f; }

// Sums of sines at unrelated frequencies: smooth, never repeating in a way the eye picks up
Vector2 ScreenShake::offset() const {
    float x = 0.6f * std::sin(time_ * 41.0f) + 0.4f * std::sin(time_ * 67.0f + 1.7f);
    float y = 0.6f * std::sin(time_ * 47.0f + 0.9f) + 0.4f * std::sin(time_ * 71.0f + 2.9f);
    return {x * MAX_OFFSET * Shake(), y * MAX_OFFSET * Shake()};
}

float ScreenShake::rotationDegrees() const {
    float r = 0.7f * std::sin(time_ * 53.0f + 0.4f) + 0.3f * std::sin(time_ * 79.0f + 2.2f);
    return r * MAX_ROTATION * Shake();
}

}  // namespace game
