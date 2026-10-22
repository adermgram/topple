#pragma once

#include "raylib.h"
#include <box2d/box2d.h>
#include <vector>

// Thin wrapper over Box2D. Everything outside this folder works in pixels
// (y down, like the screen); the conversion to metres happens here only.
namespace physics {

constexpr float PIXELS_PER_METER = 32.0f;
constexpr float GRAVITY = 18.0f;           // m/s^2, a bit above real life for snappier play
constexpr float TIME_STEP = 1.0f / 60.0f;  // fixed step keeps the simulation deterministic

inline float ToMeters(float px) { return px / PIXELS_PER_METER; }
inline b2Vec2 ToMeters(Vector2 px) { return {px.x / PIXELS_PER_METER, px.y / PIXELS_PER_METER}; }
inline Vector2 ToPixels(b2Vec2 m) { return {m.x * PIXELS_PER_METER, m.y * PIXELS_PER_METER}; }

enum class BodyType { Static, Dynamic };

struct Material {
    float density = 1.0f;
    float friction = 0.6f;
    float restitution = 0.05f;   // bounciness
};

// Two bodies colliding, and where (px). impulse is the total normal impulse of the contact (N*s):
// resting contact gives small values, a hard hit gives large ones.
struct Impact {
    const b2Body* a;
    const b2Body* b;
    float impulse;
    Vector2 point;
};

class World {
public:
    World();
    World(const World&) = delete;              // the contact listener points into this object
    World& operator=(const World&) = delete;

    // Advances the simulation in fixed steps, however long the frame took.
    // Replaces the impacts() of the previous call.
    void Update(float dt);

    // Bodies are owned by the world and live until destroyed or the world is destroyed
    b2Body* CreateBox(Vector2 centerPx, Vector2 sizePx, BodyType type, const Material& material);
    b2Body* CreateCircle(Vector2 centerPx, float radiusPx, BodyType type, const Material& material);
    void DestroyBody(b2Body* body);   // not while Update is running

    const std::vector<Impact>& impacts() const { return impacts_; }
    bool AllAsleep() const;           // true when nothing is moving any more

private:
    class ImpactListener : public b2ContactListener {
    public:
        explicit ImpactListener(std::vector<Impact>& out) : out_(out) {}
        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
    private:
        std::vector<Impact>& out_;
    };

    b2Body* CreateBody(Vector2 centerPx, BodyType type);
    static void AttachShape(b2Body* body, const b2Shape& shape, const Material& material);

    b2World world_;
    std::vector<Impact> impacts_;
    ImpactListener listener_{impacts_};
    float accumulator_ = 0.0f;
};

}  // namespace physics
