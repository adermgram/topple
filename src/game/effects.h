#pragma once

#include "raylib.h"
#include <vector>

namespace game {

// Short-lived visual effects. Purely cosmetic: they never touch the physics world.
class Effects {
public:
    // Hard little pieces that fly out and fall (debris, shards)
    void Burst(Vector2 center, Color color, int count);
    // Soft puffs that drift up, swell and fade (impact dust)
    void Dust(Vector2 center, Color color, int count);
    // A thin circle that expands and fades (a shockwave)
    void Ring(Vector2 center, Color color, float maxRadius);

    void Update(float dt);
    void Draw() const;

    int particleCount() const { return (int)particles_.size(); }

private:
    struct Particle {
        Vector2 pos;
        Vector2 vel;
        float life;       // seconds left
        float maxLife;
        float radius;
        Color color;
        float gravity;    // px/s^2, negative floats upward
        float drag;       // fraction of speed lost per second
        bool soft;        // swell and fade like dust, instead of shrinking like debris
    };

    struct Wave {
        Vector2 pos;
        float age;
        float life;
        float maxRadius;
        Color color;
    };

    std::vector<Particle> particles_;
    std::vector<Wave> waves_;
};

}  // namespace game
