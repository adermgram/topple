#include "game/feedback.h"
#include <algorithm>
#include <cmath>

namespace game {

    namespace {

        /* Collision strengths (impulses) measured in play: pieces settling stay below ~6, a standard
        // ball's hardest hit is about 41 (up to ~65), a heavy ball's about 146 (up to ~230) */
        constexpr float HIT_SOUND_MIN_IMPACT = 8.0f;
        constexpr float HIT_SOUND_FULL_VOLUME_AT = 40.0f;
        constexpr float HIT_SOUND_MIN_VOLUME = 0.3f;
        constexpr float HIT_SOUND_COOLDOWN = 0.08f;
        constexpr float HIT_SOUND_BREAKTHROUGH = 1.5f;   // a hit this much harder than the last cuts the cooldown short
        constexpr float HIT_SOUND_PITCH_VARIATION = 0.15f;

        // A typical standard-ball hit gives a trauma of 0.5, a clear thump; the heavy ball hits the cap
        constexpr float SHAKE_MIN_IMPACT = 15.0f;
        constexpr float SHAKE_REFERENCE_IMPACT = 41.0f;
        constexpr float SHAKE_REFERENCE_TRAUMA = 0.5f;
        constexpr float SHAKE_EXPONENT = 0.6f;
        constexpr float SHAKE_MAX_FROM_IMPACT = 0.9f;
        constexpr float SHAKE_PER_GLASS = 0.10f;
        constexpr float SHAKE_PER_TARGET = 0.12f;

    }  // namespace

    void Feedback::React(const LevelEvents& events, float dt) {
        shake_.Update(dt);
        hitCooldown_ -= dt;
        if (hitCooldown_ <= 0.0f) lastHitImpact_ = 0.0f;

        PlaySounds(events);
        Shake(events);
    }

    void Feedback::PlaySounds(const LevelEvents& events) {
        if (events.targetsDestroyed > 0) sound_.PlayVaried(audio::Sfx::Pop);
        if (events.glassShattered > 0) sound_.PlayVaried(audio::Sfx::Shatter);

        const float impact = events.strongestImpact;
        const bool coolingDown = hitCooldown_ > 0.0f;
        if (impact < HIT_SOUND_MIN_IMPACT) return;
        if (coolingDown && impact <= lastHitImpact_ * HIT_SOUND_BREAKTHROUGH) return;

        const float volume = std::clamp(impact / HIT_SOUND_FULL_VOLUME_AT, HIT_SOUND_MIN_VOLUME, 1.0f);
        sound_.PlayVaried(audio::Sfx::Hit, volume, HIT_SOUND_PITCH_VARIATION);
        hitCooldown_ = HIT_SOUND_COOLDOWN;
        lastHitImpact_ = impact;
    }

    /* One collision lasts several physics steps, so it raises the shake to a level instead of  adding to it each step.
    Glass and targets are one-off events and do add. */
    void Feedback::Shake(const LevelEvents& events) {
        if (events.strongestImpact >= SHAKE_MIN_IMPACT) {
            const float trauma = SHAKE_REFERENCE_TRAUMA *
                                std::pow(events.strongestImpact / SHAKE_REFERENCE_IMPACT, SHAKE_EXPONENT);
            shake_.Raise(std::min(trauma, SHAKE_MAX_FROM_IMPACT));
        }
        shake_.AddTrauma(SHAKE_PER_GLASS * static_cast<float>(events.glassShattered) +
                        SHAKE_PER_TARGET * static_cast<float>(events.targetsDestroyed));
    }

}  // namespace game
