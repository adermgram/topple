#include "game/slingshot.h"
#include "game/ball.h"
#include "physics/world.h"
#include "raymath.h"

namespace game {

namespace {

constexpr float GRAB_RADIUS = 48.0f;    // how close the cursor must be to pick up the ball
constexpr float MAX_PULL = 120.0f;
constexpr float MIN_PULL = 14.0f;       // shorter pulls cancel the shot
constexpr float POWER = 6.0f;           // launch speed (px/s) per px of pull
constexpr float RELOAD_TIME = 0.5f;
constexpr float PRONG_SPREAD = 24.0f;   // half the distance between the two tips

constexpr float TRAJECTORY_TIME = 1.6f;
constexpr float TRAJECTORY_DOT_EVERY = 0.07f;

const Color WOOD{120, 80, 52, 255};
const Color BAND{74, 40, 34, 255};

}  // namespace

Slingshot::Slingshot(Vector2 anchorPx, float groundY)
    : anchor_(anchorPx), groundY_(groundY), pull_(anchorPx) {}

Vector2 Slingshot::LaunchVelocity() const {
    return Vector2Scale(Vector2Subtract(anchor_, pull_), POWER);
}

std::optional<Slingshot::Launch> Slingshot::Update(float dt, const Pointer& pointer) {
    hot_ = false;

    switch (state_) {
    case State::Ready:
        if (!enabled_) break;
        hot_ = Vector2Distance(pointer.pos, anchor_) <= GRAB_RADIUS;
        if (hot_ && pointer.pressed) {
            state_ = State::Dragging;
            pull_ = anchor_;
        }
        break;

    case State::Dragging: {
        hot_ = true;
        Vector2 offset = Vector2Subtract(pointer.pos, anchor_);
        float len = Vector2Length(offset);
        if (len > MAX_PULL) offset = Vector2Scale(offset, MAX_PULL / len);
        pull_ = Vector2Add(anchor_, offset);

        if (pointer.released) {
            if (Vector2Length(offset) >= MIN_PULL) {
                state_ = State::Reloading;
                reloadTimer_ = RELOAD_TIME;
                return Launch{pull_, LaunchVelocity()};   // starts where the dots started
            }
            state_ = State::Ready;   // let go too close to the sling: cancel
            pull_ = anchor_;
        }
        break;
    }

    case State::Reloading:
        reloadTimer_ -= dt;
        if (reloadTimer_ <= 0.0f) {
            state_ = State::Ready;
            pull_ = anchor_;
        }
        break;
    }
    return std::nullopt;
}

void Slingshot::DrawBand(Vector2 pull) const {
    Vector2 left = {anchor_.x - PRONG_SPREAD, anchor_.y};
    Vector2 right = {anchor_.x + PRONG_SPREAD, anchor_.y};
    DrawLineEx(left, pull, 6.0f, BAND);
    DrawLineEx(right, pull, 6.0f, BAND);
}

// Follows the same discrete integration Box2D uses, so the dots match the real flight
void Slingshot::DrawTrajectory() const {
    Vector2 v = LaunchVelocity();
    float g = physics::GRAVITY * physics::PIXELS_PER_METER;
    float dt = physics::TIME_STEP;

    int dots = (int)(TRAJECTORY_TIME / TRAJECTORY_DOT_EVERY);
    for (int i = 1; i <= dots; i++) {
        float t = i * TRAJECTORY_DOT_EVERY;
        Vector2 p = {pull_.x + v.x * t, pull_.y + v.y * t + 0.5f * g * t * (t + dt)};
        if (p.y > groundY_ - Ball::RadiusOf(ball_)) break;   // the ball's centre stops a radius above the ground
        float fade = 1.0f - (float)i / (dots + 1);
        DrawCircleV(p, 4.0f * (0.5f + 0.5f * fade), Fade(RAYWHITE, 0.15f + 0.65f * fade));
    }
}

void Slingshot::Draw() const {
    // Post and the two prongs
    Vector2 fork = {anchor_.x, anchor_.y + 46.0f};
    DrawLineEx({anchor_.x, groundY_}, fork, 16.0f, WOOD);
    DrawLineEx(fork, {anchor_.x - PRONG_SPREAD, anchor_.y}, 10.0f, WOOD);
    DrawLineEx(fork, {anchor_.x + PRONG_SPREAD, anchor_.y}, 10.0f, WOOD);

    switch (state_) {
    case State::Ready:
        DrawBand(anchor_);
        if (enabled_) Ball::DrawAt(anchor_, 0.0f, ball_);
        break;
    case State::Dragging:
        if (Vector2Distance(pull_, anchor_) >= MIN_PULL) DrawTrajectory();
        DrawBand(pull_);
        Ball::DrawAt(pull_, 0.0f, ball_);
        break;
    case State::Reloading:
        DrawBand(anchor_);   // relaxed band, no ball yet
        break;
    }
}

}  // namespace game
