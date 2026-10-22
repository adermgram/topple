#pragma once

#include "app/scene_manager.h"
#include "game/progress.h"
#include "ui/theme.h"

// What every scene needs from the application. Passed by reference, owned by App.
struct AppContext {
    ui::Theme& theme;
    SceneManager& scenes;
    game::Progress& progress;
};
