#include "audio/sound_bank.h"
#include "audio/synth.h"

namespace audio {

namespace {

constexpr float MASTER_VOLUME = 0.7f;

Samples Generate(Sfx sfx) {
    switch (sfx) {
    case Sfx::Click:   return MakeClick();
    case Sfx::Launch:  return MakeLaunch();
    case Sfx::Hit:     return MakeHit();
    case Sfx::Pop:     return MakePop();
    case Sfx::Shatter: return MakeShatter();
    case Sfx::Split:   return MakeSplit();
    case Sfx::Win:     return MakeWin();
    case Sfx::Lose:    return MakeLose();
    case Sfx::Count:   break;
    }
    return {};
}

}  // namespace

SoundBank::SoundBank() {
    ready_ = IsAudioDeviceReady();
    if (!ready_) return;
    ApplyMasterVolume();

    for (std::size_t i = 0; i < effects_.size(); i++) {
        Samples samples = Generate(static_cast<Sfx>(i));
        Wave wave{static_cast<unsigned int>(samples.size()), SAMPLE_RATE, 16, 1, samples.data()};

        // LoadSoundFromWave copies the samples
        Effect& effect = effects_[i];
        effect.voices[0] = LoadSoundFromWave(wave);
        for (std::size_t v = 1; v < VOICES; v++) effect.voices[v] = LoadSoundAlias(effect.voices[0]);
    }
}

SoundBank::~SoundBank() {
    if (!ready_) return;
    for (Effect& effect : effects_) {
        for (std::size_t v = 1; v < VOICES; v++) UnloadSoundAlias(effect.voices[v]);
        UnloadSound(effect.voices[0]);   // owns the samples, so it goes last
    }
}

void SoundBank::Play(Sfx sfx, float volume, float pitch) {
    if (!ready_) return;
    Effect& effect = effects_[static_cast<std::size_t>(sfx)];
    Sound& voice = effect.voices[effect.next];
    effect.next = (effect.next + 1) % VOICES;

    SetSoundVolume(voice, volume);
    SetSoundPitch(voice, pitch);
    PlaySound(voice);
}

void SoundBank::PlayVaried(Sfx sfx, float volume, float variation) {
    float jitter = static_cast<float>(GetRandomValue(-1000, 1000)) / 1000.0f * variation;
    Play(sfx, volume, 1.0f + jitter);
}

void SoundBank::ToggleMute() {
    muted_ = !muted_;
    if (ready_) ApplyMasterVolume();
}

void SoundBank::ApplyMasterVolume() const { SetMasterVolume(muted_ ? 0.0f : MASTER_VOLUME); }

}  // namespace audio
