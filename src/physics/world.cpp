#include "physics/world.h"

namespace physics {

    namespace {

        constexpr int VELOCITY_ITERATIONS = 8;
        constexpr int POSITION_ITERATIONS = 3;
        constexpr int MAX_STEPS_PER_FRAME = 5;   
        constexpr float ROLLING_DAMPING = 0.8f;

        // Contacts softer than this are not reported: resting blocks touch every step, and nothing
        // reacts below it (the lowest threshold in the game is a target's, at 4)
        constexpr float MIN_REPORTED_IMPULSE = 2.0f;

    }  // namespace

    World::World() : world_(b2Vec2(0.0f, GRAVITY)) {
        world_.SetContactListener(&listener_);
    }

    void World::ImpactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
        float total = 0.0f;
        for (int i = 0; i < impulse->count; i++) total += impulse->normalImpulses[i];
        if (total < MIN_REPORTED_IMPULSE) return;

        b2Body* bodyA = contact->GetFixtureA()->GetBody();
        Vector2 point = ToPixels(bodyA->GetPosition());   
        if (contact->GetManifold()->pointCount > 0) {
            b2WorldManifold manifold;
            contact->GetWorldManifold(&manifold);
            point = ToPixels(manifold.points[0]);
        }
        out_.push_back({bodyA, contact->GetFixtureB()->GetBody(), total, point});
    }

    void World::DestroyBody(b2Body* body) {
        previousPoses_.erase(body);
        world_.DestroyBody(body);
    }

    void World::RememberPoses() {
        for (const b2Body* body = world_.GetBodyList(); body; body = body->GetNext())
            if (body->GetType() == b2_dynamicBody)
                previousPoses_[body] = {ToPixels(body->GetPosition()), body->GetAngle()};
    }

    Pose World::DrawPose(const b2Body* body) const {
        const Pose now{ToPixels(body->GetPosition()), body->GetAngle()};
        const auto previous = previousPoses_.find(body);
        if (previous == previousPoses_.end()) return now;   // static, or created since the last step

        const float t = accumulator_ / TIME_STEP;
        const Pose& before = previous->second;
        return {{before.position.x + (now.position.x - before.position.x) * t,
                 before.position.y + (now.position.y - before.position.y) * t},
                before.angle + (now.angle - before.angle) * t};
    }

    bool World::allAsleep() const {
        for (const b2Body* body = world_.GetBodyList(); body; body = body->GetNext())
            if (body->GetType() == b2_dynamicBody && body->IsAwake()) return false;
        return true;
    }

    void World::Update(float dt) {
        impacts_.clear();
        accumulator_ += dt;
        int steps = 0;
        while (accumulator_ >= TIME_STEP && steps < MAX_STEPS_PER_FRAME) {
            RememberPoses();
            world_.Step(TIME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
            accumulator_ -= TIME_STEP;
            steps++;
        }
        if (steps == MAX_STEPS_PER_FRAME) accumulator_ = 0.0f;
    }

    b2Body* World::CreateBody(Vector2 centerPx, BodyType type) {
        b2BodyDef bodyDef;
        bodyDef.type = (type == BodyType::Static) ? b2_staticBody : b2_dynamicBody;
        bodyDef.position = ToMeters(centerPx);
        return world_.CreateBody(&bodyDef);
    }

    void World::AttachShape(b2Body* body, const b2Shape& shape, const Material& material) {
        b2FixtureDef fixture;
        fixture.shape = &shape;
        fixture.density = material.density;
        fixture.friction = material.friction;
        fixture.restitution = material.restitution;
        body->CreateFixture(&fixture);
    }

    b2Body* World::CreateBox(Vector2 centerPx, Vector2 sizePx, BodyType type, const Material& material) {
        b2Body* body = CreateBody(centerPx, type);
        b2PolygonShape shape;
        shape.SetAsBox(ToMeters(sizePx.x) / 2, ToMeters(sizePx.y) / 2);
        AttachShape(body, shape, material);
        return body;
    }

    b2Body* World::CreateCircle(Vector2 centerPx, float radiusPx, BodyType type, const Material& material) {
        b2Body* body = CreateBody(centerPx, type);
        // Box2D has no rolling resistance: without this a ball would roll on flat ground forever
        body->SetAngularDamping(ROLLING_DAMPING);

        b2CircleShape shape;
        shape.m_radius = ToMeters(radiusPx);
        AttachShape(body, shape, material);
        return body;
    }

}  // namespace physics
