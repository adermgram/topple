#include "game/effects.h"
#include <algorithm>
#include <cmath>

namespace game {

namespace {

constexpr float PARTICLE_GRAVITY = 500.0f;   // px/s^2

float RandomFloat(float lo, float hi) {
    return lo + (hi - lo) * (GetRandomValue(0, 10000) / 10000.0f);
}

}  // namespace

void Effects::Burst(Vector2 center, Color color, int count) {
    for (int i = 0; i < count; i++) {
        float angle = RandomFloat(0.0f, 2.0f * PI);
        float speed = RandomFloat(80.0f, 280.0f);
        float life = RandomFloat(0.4f, 0.9f);
        particles_.push_back({center,
                              {std::cos(angle) * speed, std::sin(angle) * speed - 80.0f},
                              life, life, RandomFloat(3.0f, 7.0f), color});
    }
}

void Effects::Update(float dt) {
    for (Particle& p : particles_) {
        p.vel.y += PARTICLE_GRAVITY * dt;
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.life -= dt;
    }
    particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                    [](const Particle& p) { return p.life <= 0.0f; }),
                     particles_.end());
}

void Effects::Draw() const {
    for (const Particle& p : particles_) {
        float t = p.life / p.maxLife;   // 1 -> 0
        DrawCircleV(p.pos, p.radius * (0.4f + 0.6f * t), Fade(p.color, t));
    }
}

}  // namespace game
