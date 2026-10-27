#pragma once

#include <cstdint>
#include <vector>

/* The game's sound effects, generated from simple oscillators and noise instead of loaded from files. 
Plain functions with no raylib dependency, so they can be tested alone. */
namespace audio {

    constexpr int SAMPLE_RATE = 44100;

    using Samples = std::vector<std::int16_t>;   // mono, 16-bit

    Samples MakeClick();    // button press
    Samples MakeLaunch();   // slingshot release
    Samples MakeHit();      // something hits something: a low thud
    Samples MakePop();      // a target is destroyed
    Samples MakeShatter();  // a glass block breaks
    Samples MakeSplit();    // a splitter ball divides in three
    Samples MakeWin();      // rising arpeggio
    Samples MakeLose();     // falling notes

}  // namespace audio
