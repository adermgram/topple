#pragma once

#include "app/app_context.h"
#include "app/scene_manager.h"
#include "game/progress.h"
#include "ui/theme.h"

class App {
public:
    App();
    void Run();

private:
    // Declared first so the window exists before the theme loads fonts,
    // and closes after everything that holds GPU resources is gone.
    struct WindowGuard {
        WindowGuard();
        ~WindowGuard();
    };

    WindowGuard window_;
    ui::Theme theme_;
    SceneManager scenes_;
    game::Progress progress_;
    AppContext context_{theme_, scenes_, progress_};
};
