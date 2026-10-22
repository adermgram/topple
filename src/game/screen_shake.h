#pragma once

#include "raylib.h"

namespace game {

// Camera shake with the "trauma" model: hits add trauma (0..1), it fades steadily, and the
// shake grows with the square of it, so small bumps barely show and big hits really rattle.
class ScreenShake {
public:
    void AddTrauma(float amount);   // stacks, capped at 1
    void Raise(float level);        // at least this much; does not stack (for one event seen over several frames)
    void Update(float dt);
    void Reset();

    Vector2 offset() const;          // px
    float rotationDegrees() const;
    float trauma() const { return trauma_; }

private:
    float Shake() const { return trauma_ * trauma_; }

    float trauma_ = 0.0f;
    float time_ = 0.0f;   // drives the wobble; not reset, so each shake differs
};

}  // namespace game
