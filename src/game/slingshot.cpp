#include "game/slingshot.h"
#include "game/ball.h"
#include "physics/world.h"
#include "raymath.h"

namespace game {

namespace {

constexpr float GRAB_RADIUS = 48.0f;    // how close the cursor must be to pick the ball up
constexpr float MAX_PULL = 120.0f;
constexpr float MIN_PULL = 14.0f;       // a shorter pull cancels the shot
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
        const float length = Vector2Length(offset);
        if (length > MAX_PULL) offset = Vector2Scale(offset, MAX_PULL / length);
        pull_ = Vector2Add(anchor_, offset);

        if (pointer.released) {
            if (Vector2Length(offset) >= MIN_PULL) {
                state_ = State::Reloading;
                reloadTimer_ = RELOAD_TIME;
                return Launch{pull_, LaunchVelocity()};
            }
            Cancel();
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

void Slingshot::Cancel() {
    if (state_ != State::Dragging) return;
    state_ = State::Ready;
    pull_ = anchor_;
    hot_ = false;
}

void Slingshot::DrawBand(Vector2 pull) const {
    DrawLineEx({anchor_.x - PRONG_SPREAD, anchor_.y}, pull, 6.0f, BAND);
    DrawLineEx({anchor_.x + PRONG_SPREAD, anchor_.y}, pull, 6.0f, BAND);
}

// Uses the same discrete integration as Box2D, so the dots land where the ball does
void Slingshot::DrawTrajectory() const {
    const Vector2 v = LaunchVelocity();
    const float g = physics::GRAVITY * physics::PIXELS_PER_METER;
    const float dt = physics::TIME_STEP;

    const int dots = static_cast<int>(TRAJECTORY_TIME / TRAJECTORY_DOT_EVERY);
    for (int i = 1; i <= dots; i++) {
        const float t = static_cast<float>(i) * TRAJECTORY_DOT_EVERY;
        const Vector2 p = {pull_.x + v.x * t, pull_.y + v.y * t + 0.5f * g * t * (t + dt)};
        if (p.y > groundY_ - Ball::RadiusOf(ball_)) break;

        const float fade = 1.0f - static_cast<float>(i) / static_cast<float>(dots + 1);
        DrawCircleV(p, 4.0f * (0.5f + 0.5f * fade), Fade(RAYWHITE, 0.15f + 0.65f * fade));
    }
}

void Slingshot::Draw() const {
    const Vector2 fork = {anchor_.x, anchor_.y + 46.0f};
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
        DrawBand(anchor_);
        break;
    }
}

}  // namespace game
