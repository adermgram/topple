#pragma once

#include "raylib.h"
#include <vector>

namespace game {

// Short-lived visual particles. Purely cosmetic: they never touch the physics world.
class Effects {
public:
    void Burst(Vector2 center, Color color, int count);
    void Update(float dt);
    void Draw() const;

private:
    struct Particle {
        Vector2 pos;
        Vector2 vel;
        float life;       // seconds left
        float maxLife;
        float radius;
        Color color;
    };

    std::vector<Particle> particles_;
};

}  // namespace game
