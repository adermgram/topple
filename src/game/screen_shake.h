#pragma once

#include "raylib.h"

namespace game {

    /* Camera shake driven by "trauma" (0..1): hits add to it, it fades at a steady rate, and the
     shake grows with its square, so small bumps barely show and big hits really rattle. */
    class ScreenShake {
    public:
        void AddTrauma(float amount);   
        void Raise(float level);
        void Update(float dt);
        void Reset();

        Vector2 offset() const;
        float rotationDegrees() const;

    private:
        float Shake() const { return trauma_ * trauma_; }

        float trauma_ = 0.0f;
        float time_ = 0.0f;
    };

}  // namespace game
