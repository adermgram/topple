#pragma once

#include "audio/sound_bank.h"
#include "game/level_events.h"
#include "game/screen_shake.h"

namespace game {

// Turns what happened in a level into sounds and camera shake
class Feedback {
public:
    explicit Feedback(audio::SoundBank& sound) : sound_(sound) {}

    // Call once per frame the world runs, with that frame's events
    void React(const LevelEvents& events, float dt);
    void Reset() { shake_.Reset(); }

    const ScreenShake& shake() const { return shake_; }

private:
    void PlaySounds(const LevelEvents& events);
    void Shake(const LevelEvents& events);

    audio::SoundBank& sound_;
    ScreenShake shake_;
    float hitCooldown_ = 0.0f;     // seconds until another hit sound may play
    float lastHitImpact_ = 0.0f;   // impulse of the hit that started the cooldown
};

}  // namespace game
