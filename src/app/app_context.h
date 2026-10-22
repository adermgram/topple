#pragma once

class SceneManager;

namespace audio { class SoundBank; }
namespace game { class Progress; }
namespace ui { class Theme; }

// What every scene needs from the application. Owned by App, passed around by reference.
struct AppContext {
    ui::Theme& theme;
    SceneManager& scenes;
    game::Progress& progress;
    audio::SoundBank& sound;
};
