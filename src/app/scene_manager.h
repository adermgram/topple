#pragma once

#include "app/scene.h"
#include <memory>
#include <utility>

// Holds the active scene. A replacement only takes effect at the start of the next Update,
// so a scene can ask to be replaced from inside its own Update.
class SceneManager {
public:
    void Replace(std::unique_ptr<Scene> scene) { pending_ = std::move(scene); }
    void Quit() { quit_ = true; }
    bool quitRequested() const { return quit_; }

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
