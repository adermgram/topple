#include "audio/synth.h"
#include <algorithm>
#include <cmath>

namespace audio {

namespace {

constexpr float TWO_PI = 6.2831853f;

constexpr float INAUDIBLE = 0.002f;   // an envelope below this is treated as finished

using Buffer = std::vector<float>;   // built as floats, then scaled to a target peak

std::size_t SampleAt(float seconds) { return static_cast<std::size_t>(seconds * SAMPLE_RATE); }

float TimeOf(std::size_t sample) { return static_cast<float>(sample) / SAMPLE_RATE; }

Buffer Silence(float seconds) { return Buffer(SampleAt(seconds), 0.0f); }

// Deterministic white noise, so every run sounds the same
class Noise {
public:
    float Next() {
        state_ ^= state_ << 13;
        state_ ^= state_ >> 17;
        state_ ^= state_ << 5;
        return static_cast<float>(state_) / 2147483648.0f - 1.0f;
    }

private:
    std::uint32_t state_ = 0x12345678u;
};

// One-pole low-pass filter. The cutoff can move while it runs (a sweep).
class LowPass {
public:
    explicit LowPass(float cutoffHz) { SetCutoff(cutoffHz); }
    void SetCutoff(float cutoffHz) { coefficient_ = 1.0f - std::exp(-TWO_PI * cutoffHz / SAMPLE_RATE); }
    float Process(float x) { return state_ += coefficient_ * (x - state_); }

private:
    float coefficient_ = 1.0f;
    float state_ = 0.0f;
};

// A note: a sine with two quiet overtones, a short attack and an exponential decay
void AddTone(Buffer& out, float start, float duration, float freq, float gain, float decay) {
    constexpr float ATTACK = 0.006f;
    constexpr float RELEASE = 0.02f;   // fades the very end so the note can't click off

    const std::size_t first = SampleAt(start);
    const std::size_t count = SampleAt(duration);
    for (std::size_t i = 0; i < count && first + i < out.size(); i++) {
        float t = TimeOf(i);
        float envelope = std::min(1.0f, t / ATTACK) * std::exp(-decay * t) *
                         std::min(1.0f, (duration - t) / RELEASE);
        float phase = TWO_PI * freq * t;
        float wave = std::sin(phase) + 0.25f * std::sin(2 * phase) + 0.1f * std::sin(3 * phase);
        out[first + i] += gain * envelope * wave;
    }
}

// A struck piece of wood. Real objects don't ring at one pitch: they vibrate in several
// modes at uneven frequency ratios, the higher ones dying fastest. That uneven stack is
// what makes a knock sound like wood and not like a beep.
void AddStrike(Buffer& out, float baseFreq, float gain, float decayScale) {
    struct Mode {
        float ratio, gain, decay;
    };
    constexpr Mode MODES[] = {{1.00f, 1.00f, 24.0f}, {2.76f, 0.60f, 40.0f},
                              {5.40f, 0.35f, 75.0f}, {8.93f, 0.20f, 120.0f}};

    for (const Mode& mode : MODES) {
        for (std::size_t i = 0; i < out.size(); i++) {
            float t = TimeOf(i);
            float envelope = std::exp(-mode.decay * decayScale * t);
            if (envelope < INAUDIBLE) break;
            out[i] += gain * mode.gain * envelope * std::sin(TWO_PI * baseFreq * mode.ratio * t);
        }
    }
}

// A short ring. Several at different pitches and start times make falling glass tinkle.
void AddPing(Buffer& out, float start, float freq, float gain, float decay) {
    const std::size_t first = SampleAt(start);
    for (std::size_t i = first; i < out.size(); i++) {
        float t = TimeOf(i - first);
        float envelope = std::exp(-decay * t);
        if (envelope < INAUDIBLE) break;
        out[i] += gain * envelope * std::sin(TWO_PI * freq * t);
    }
}

// A rising "pew": the pitch glides from f0 toward f1 while the sound dies away
void AddChirp(Buffer& out, float start, float f0, float f1, float gain, float decay) {
    const std::size_t first = SampleAt(start);
    float phase = 0.0f;
    for (std::size_t i = first; i < out.size(); i++) {
        float t = TimeOf(i - first);
        float envelope = std::exp(-decay * t);
        if (envelope < INAUDIBLE) break;
        phase += TWO_PI * (f0 + (f1 - f0) * (1.0f - std::exp(-35.0f * t))) / SAMPLE_RATE;
        out[i] += gain * envelope * std::sin(phase);
    }
}

// Scales to an exact peak, then fades in over 0.2 ms and out over 20 ms, so a sound that
// doesn't start or end exactly at zero can't click
Samples ToPcm(const Buffer& in, float peak) {
    constexpr float FADE_IN = 0.0002f;
    constexpr float FADE_OUT = 0.02f;
    const float total = TimeOf(in.size());

    float loudest = 0.0f;
    for (float v : in) loudest = std::max(loudest, std::fabs(v));
    const float scale = loudest > 0.0f ? peak / loudest : 0.0f;

    Samples out(in.size());
    for (std::size_t i = 0; i < in.size(); i++) {
        float t = TimeOf(i);
        float fade = std::min(1.0f, t / FADE_IN) * std::min(1.0f, (total - t) / FADE_OUT);
        out[i] = static_cast<std::int16_t>(std::clamp(in[i] * scale * fade, -1.0f, 1.0f) * 32767.0f);
    }
    return out;
}

}  // namespace

Samples MakeClick() {
    Buffer b = Silence(0.07f);
    for (std::size_t i = 0; i < b.size(); i++) {
        float t = TimeOf(i);
        b[i] = std::sin(TWO_PI * 1100.0f * t) * std::exp(-90.0f * t);
    }
    return ToPcm(b, 0.45f);
}

// A rubber band letting go: a bright snap, a short falling twang, and the rush of air
// as the ball leaves
Samples MakeLaunch() {
    Buffer b = Silence(0.4f);
    Noise noise;
    LowPass airHigh(2500.0f), airLow(500.0f);   // high minus low leaves a band of noise
    float twangPhase = 0.0f, overtonePhase = 0.0f;

    for (std::size_t i = 0; i < b.size(); i++) {
        float t = TimeOf(i);

        float snap = noise.Next() * std::exp(-220.0f * t);

        // The pitch falls fast, with a slight wobble like a vibrating band
        float freq = (140.0f + 300.0f * std::exp(-14.0f * t)) * (1.0f + 0.03f * std::sin(TWO_PI * 28.0f * t));
        twangPhase += TWO_PI * freq / SAMPLE_RATE;
        overtonePhase += TWO_PI * freq * 2.01f / SAMPLE_RATE;
        float twang = (std::sin(twangPhase) + 0.4f * std::sin(overtonePhase)) * std::exp(-10.0f * t);

        float air = airHigh.Process(noise.Next()) - airLow.Process(noise.Next());
        float rush = air * (1.0f - std::exp(-60.0f * t)) * std::exp(-7.0f * t);

        b[i] = 0.8f * snap + 0.5f * twang + 1.6f * rush;
    }
    return ToPcm(b, 0.7f);
}

// Wood on wood: a sharp click, a knock from several uneven tones, and a dull thump for weight
Samples MakeHit() {
    Buffer b = Silence(0.25f);
    Noise noise;
    LowPass click(5000.0f);
    float thumpPhase = 0.0f;

    AddStrike(b, 190.0f, 0.7f, 1.0f);
    for (std::size_t i = 0; i < b.size(); i++) {
        float t = TimeOf(i);
        thumpPhase += TWO_PI * (60.0f + 70.0f * std::exp(-35.0f * t)) / SAMPLE_RATE;
        b[i] += 0.6f * std::sin(thumpPhase) * std::exp(-30.0f * t);
        b[i] += 0.5f * click.Process(noise.Next()) * std::exp(-300.0f * t);
    }
    return ToPcm(b, 0.9f);
}

// A soft "poof" of air with a tiny squeak on top and a click to start it
Samples MakePop() {
    Buffer b = Silence(0.3f);
    Noise noise;
    LowPass puff(3000.0f);
    float chirpPhase = 0.0f;

    for (std::size_t i = 0; i < b.size(); i++) {
        float t = TimeOf(i);

        puff.SetCutoff(500.0f + 2500.0f * std::exp(-25.0f * t));   // duller as it fades
        float poof = puff.Process(noise.Next()) * std::exp(-16.0f * t);

        chirpPhase += TWO_PI * (350.0f + 650.0f * (1.0f - std::exp(-55.0f * t))) / SAMPLE_RATE;
        float squeak = std::sin(chirpPhase) * std::exp(-38.0f * t);

        float click = noise.Next() * std::exp(-400.0f * t);

        b[i] = 1.4f * poof + 0.45f * squeak + 0.3f * click;
    }
    return ToPcm(b, 0.75f);
}

// A crack and a hiss of high noise, then pieces tinkling at scattered pitches
Samples MakeShatter() {
    Buffer b = Silence(0.5f);
    Noise noise;
    LowPass split(3500.0f);   // noise minus its low part is a bright hiss

    for (std::size_t i = 0; i < b.size(); i++) {
        float t = TimeOf(i);
        float raw = noise.Next();
        float hiss = (raw - split.Process(raw)) * std::exp(-14.0f * t);
        float crack = noise.Next() * std::exp(-500.0f * t);
        b[i] += 0.9f * hiss + 0.7f * crack;
    }

    struct Ping {
        float start, freq, gain, decay;
    };
    constexpr Ping PINGS[] = {{0.000f, 3150.0f, 0.50f, 38.0f}, {0.012f, 4200.0f, 0.40f, 45.0f},
                              {0.028f, 2650.0f, 0.45f, 34.0f}, {0.045f, 5300.0f, 0.30f, 55.0f},
                              {0.070f, 3700.0f, 0.35f, 40.0f}, {0.100f, 4800.0f, 0.25f, 50.0f},
                              {0.140f, 6100.0f, 0.20f, 60.0f}};
    for (const Ping& p : PINGS) AddPing(b, p.start, p.freq, p.gain, p.decay);
    return ToPcm(b, 0.8f);
}

// Three overlapping chirps, one per new ball, over a small burst of noise
Samples MakeSplit() {
    Buffer b = Silence(0.25f);
    Noise noise;
    for (std::size_t i = 0; i < b.size(); i++) b[i] = 0.25f * noise.Next() * std::exp(-300.0f * TimeOf(i));

    AddChirp(b, 0.000f, 520.0f, 1500.0f, 0.60f, 22.0f);
    AddChirp(b, 0.035f, 600.0f, 1750.0f, 0.50f, 22.0f);
    AddChirp(b, 0.070f, 700.0f, 2000.0f, 0.40f, 22.0f);
    return ToPcm(b, 0.6f);
}

// C major arpeggio, the last note held
Samples MakeWin() {
    Buffer b = Silence(1.3f);
    const float notes[] = {523.25f, 659.25f, 783.99f, 1046.50f};
    for (int i = 0; i < 4; i++) {
        bool last = (i == 3);
        AddTone(b, static_cast<float>(i) * 0.11f, last ? 0.85f : 0.45f, notes[i], 0.32f, last ? 3.0f : 5.0f);
    }
    return ToPcm(b, 0.55f);
}

// Four notes stepping down
Samples MakeLose() {
    Buffer b = Silence(1.2f);
    const float notes[] = {392.00f, 349.23f, 311.13f, 261.63f};
    for (int i = 0; i < 4; i++) AddTone(b, static_cast<float>(i) * 0.2f, 0.5f, notes[i], 0.35f, 6.0f);
    return ToPcm(b, 0.45f);
}

}  // namespace audio
