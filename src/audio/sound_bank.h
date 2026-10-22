#pragma once

#include "raylib.h"
#include <array>

namespace audio {

enum class Sfx { Click, Launch, Hit, Pop, Shatter, Split, Win, Lose, Count };

// Builds every sound effect once at startup and plays them on request.
// Needs the audio device to be open (see App). If there is no audio device the
// game simply stays silent.
class SoundBank {
public:
    SoundBank();
    ~SoundBank();
    SoundBank(const SoundBank&) = delete;
    SoundBank& operator=(const SoundBank&) = delete;

    void Play(Sfx sfx, float volume = 1.0f, float pitch = 1.0f);
    // Same, with the pitch nudged randomly by up to +/- variation (0.1 = 10 %)
    // so repeated sounds don't feel mechanical
    void PlayVaried(Sfx sfx, float volume = 1.0f, float variation = 0.08f);

    void ToggleMute();
    bool muted() const { return muted_; }

private:
    // Quick repeats must overlap instead of cutting each other off, so each effect
    // has a few "voices": aliases that share one set of samples
    static constexpr int VOICES = 4;

    struct Effect {
        std::array<Sound, VOICES> voices{};
        int next = 0;
    };

    void ApplyMasterVolume() const;

    std::array<Effect, (std::size_t)Sfx::Count> effects_;
    bool ready_ = false;
    bool muted_ = false;
};

}  // namespace audio
