#pragma once

#include "raylib.h"
#include <array>
#include <cstddef>

namespace audio {

    enum class Sfx { Click, Launch, Hit, Pop, Shatter, Split, Win, Lose, Count };

    // Builds every effect once at startup and plays them on request. Without an audio device the game stays silent.
    
    class SoundBank {
    public:
        SoundBank();
        ~SoundBank();
        SoundBank(const SoundBank&) = delete;
        SoundBank& operator=(const SoundBank&) = delete;

        void Play(Sfx sfx, float volume = 1.0f, float pitch = 1.0f);
        // Nudges the pitch by up to +/- variation (0.1 = 10 %) so repeats don't sound mechanical
        void PlayVaried(Sfx sfx, float volume = 1.0f, float variation = 0.08f);

        void ToggleMute();

    private:
        // Each effect has a few voices (aliases sharing one set of samples), so quick repeats overlap instead of cutting each other off
        static constexpr std::size_t VOICES = 4;

        struct Effect {
            std::array<Sound, VOICES> voices{};
            std::size_t next = 0;
        };

        void ApplyMasterVolume() const;

        std::array<Effect, static_cast<std::size_t>(Sfx::Count)> effects_;
        bool ready_ = false;
        bool muted_ = false;
    };

}  // namespace audio
