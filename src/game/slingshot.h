#pragma once

#include "app/pointer.h"
#include "game/ball_kind.h"
#include "raylib.h"
#include <optional>

namespace game {

// Drag the ball back and release to fire. Doesn't know about the physics world:
// Update just reports the launch velocity and the owner creates the ball.
class Slingshot {
public:
    // Where a released ball starts and how fast it goes (px, px/s)
    struct Launch {
        Vector2 position;
        Vector2 velocity;
    };

    // anchor: where the ball rests; groundY: where the post meets the ground
    Slingshot(Vector2 anchorPx, float groundY);

    // Returns the launch on the frame the ball is released
    std::optional<Launch> Update(float dt, const Pointer& pointer);
    void Draw() const;

    // Which kind of ball is loaded: it is drawn in the sling and sets the trajectory's end
    void set_ball(BallKind kind) { ball_ = kind; }

    // Puts a half-pulled ball back, as if the player had let go too close to the sling
    void Cancel();

    // A disabled sling shows no ball and can't be grabbed (out of shots, round over)
    void set_enabled(bool enabled) { enabled_ = enabled; }

    Vector2 anchor() const { return anchor_; }
    bool WantsPointer() const { return hot_; }   // cursor is over the ball or dragging it

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
    Vector2 pull_;           // ball position while dragging
    float reloadTimer_ = 0.0f;
    bool hot_ = false;
};

}  // namespace game
