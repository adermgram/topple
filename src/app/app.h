#pragma once

#include "app/app_context.h"
#include "app/scene_manager.h"
#include "audio/sound_bank.h"
#include "game/progress.h"
#include "ui/theme.h"

class App {
public:
    App();
    void Run();

private:
    // Members are created top to bottom and destroyed in reverse: the window has to exist
    // before the theme loads fonts, and the audio device before the sounds are built
    struct WindowGuard {
        WindowGuard();
        ~WindowGuard();
    };
    struct AudioGuard {
        AudioGuard();
        ~AudioGuard();
    };

    WindowGuard window_;
    AudioGuard audio_;
    ui::Theme theme_;
    SceneManager scenes_;
    game::Progress progress_;
    audio::SoundBank sound_;
    AppContext context_{theme_, scenes_, progress_, sound_};
};
