#pragma once

#include "app/scene.h"
#include <memory>

// Holds the active scene. Replacing is deferred to the start of the next update,
// so a scene can safely ask to be replaced from inside its own Update.
class SceneManager {
public:
    void Replace(std::unique_ptr<Scene> scene) { pending_ = std::move(scene); }
    void Quit() { quit_ = true; }
    bool QuitRequested() const { return quit_; }

    void Update(float dt) {
        if (pending_) current_ = std::move(pending_);
        if (current_) current_->Update(dt);
    }
    void Draw() const {
        if (current_) current_->Draw();
    }

private:
    std::unique_ptr<Scene> current_;
    std::unique_ptr<Scene> pending_;
    bool quit_ = false;
};
