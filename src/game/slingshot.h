#pragma once

#include "game/ball_kind.h"
#include "input/pointer.h"
#include "raylib.h"
#include <optional>

namespace game {

// Drag the ball back and release to fire. The slingshot knows nothing about the physics
// world: Update reports the launch and the caller creates the ball.
class Slingshot {
public:
    struct Launch {
        Vector2 position;   // px
        Vector2 velocity;   // px/s
    };

    // anchor is where the ball rests, groundY where the post meets the ground
    Slingshot(Vector2 anchorPx, float groundY);

    // Returns the launch on the frame the ball is released
    std::optional<Launch> Update(float dt, const Pointer& pointer);
    void Draw() const;

    // Puts a half-pulled ball back, as if it had been let go too close to the sling
    void Cancel();

    void setBall(BallKind kind) { ball_ = kind; }
    // A disabled sling shows no ball and can't be grabbed
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool wantsPointer() const { return hot_; }   // the cursor is over the ball or dragging it

private:
    enum class State { Ready, Dragging, Reloading };

    Vector2 LaunchVelocity() const;
    void DrawTrajectory() const;
    void DrawBand(Vector2 pull) const;

    Vector2 anchor_;
    float groundY_;
    State state_ = State::Ready;
    bool enabled_ = true;
    BallKind ball_ = BallKind::Standard;
    Vector2 pull_;   // where the ball is while dragging
    float reloadTimer_ = 0.0f;
    bool hot_ = false;
};

}  // namespace game
