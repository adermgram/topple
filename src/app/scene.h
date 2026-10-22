#pragma once

// One screen of the game (menu, level, ...). The SceneManager owns the active one.
class Scene {
public:
    virtual ~Scene() = default;
    virtual void Update(float dt) = 0;
    virtual void Draw() const = 0;
};
