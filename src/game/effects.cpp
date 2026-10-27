#include "game/effects.h"
#include <algorithm>
#include <cmath>

namespace game {

    namespace {

        constexpr float DEBRIS_GRAVITY = 500.0f;   // px/s^2
        constexpr float DUST_RISE = -30.0f;        // dust drifts slightly upward
        constexpr float DUST_DRAG = 3.0f;
        constexpr float WAVE_SECONDS = 0.35f;

        float RandomFloat(float lo, float hi) {
            return lo + (hi - lo) * (static_cast<float>(GetRandomValue(0, 10000)) / 10000.0f);
        }

    }  // namespace

    void Effects::Burst(Vector2 center, Color color, int count) {
        for (int i = 0; i < count; i++) {
            float angle = RandomFloat(0.0f, 2.0f * PI);
            float speed = RandomFloat(80.0f, 280.0f);
            float life = RandomFloat(0.4f, 0.9f);
            particles_.push_back({center,
                                {std::cos(angle) * speed, std::sin(angle) * speed - 80.0f},
                                life, life, RandomFloat(3.0f, 7.0f), color, DEBRIS_GRAVITY, 0.0f, false});
        }
    }

    void Effects::Dust(Vector2 center, Color color, int count) {
        for (int i = 0; i < count; i++) {
            float angle = RandomFloat(0.0f, 2.0f * PI);
            float speed = RandomFloat(25.0f, 110.0f);
            float life = RandomFloat(0.35f, 0.75f);
            Vector2 start = {center.x + RandomFloat(-4.0f, 4.0f), center.y + RandomFloat(-4.0f, 4.0f)};
            particles_.push_back({start,
                                {std::cos(angle) * speed, std::sin(angle) * speed * 0.6f - 20.0f},
                                life, life, RandomFloat(4.0f, 8.0f), color, DUST_RISE, DUST_DRAG, true});
        }
    }

    void Effects::Ring(Vector2 center, Color color, float maxRadius) {
        waves_.push_back({center, 0.0f, WAVE_SECONDS, maxRadius, color});
    }

    void Effects::Update(float dt) {
        for (Particle& p : particles_) {
            p.vel.y += p.gravity * dt;
            float keep = std::max(0.0f, 1.0f - p.drag * dt);
            p.vel.x *= keep;
            p.vel.y *= keep;
            p.pos.x += p.vel.x * dt;
            p.pos.y += p.vel.y * dt;
            p.life -= dt;
        }
        particles_.erase(std::remove_if(particles_.begin(), particles_.end(),
                                        [](const Particle& p) { return p.life <= 0.0f; }),
                        particles_.end());

        for (Wave& w : waves_) w.age += dt;
        waves_.erase(std::remove_if(waves_.begin(), waves_.end(),
                                    [](const Wave& w) { return w.age >= w.life; }),
                    waves_.end());
    }

    void Effects::Draw() const {
        for (const Particle& p : particles_) {
            float t = p.life / p.maxLife;
            if (p.soft) DrawCircleV(p.pos, p.radius * (1.0f + 0.9f * (1.0f - t)), Fade(p.color, 0.45f * t));
            else DrawCircleV(p.pos, p.radius * (0.4f + 0.6f * t), Fade(p.color, t));
        }

        for (const Wave& w : waves_) {
            float t = w.age / w.life;
            float eased = 1.0f - (1.0f - t) * (1.0f - t);   // fast at first, then slows
            float radius = w.maxRadius * eased;
            DrawRing(w.pos, std::max(0.0f, radius - 3.0f), radius, 0.0f, 360.0f, 40, Fade(w.color, 0.8f * (1.0f - t)));
        }
    }

}  // namespace game
