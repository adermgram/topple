#pragma once

#include "raylib.h"
#include <box2d/box2d.h>
#include <vector>

// A thin wrapper over Box2D. The rest of the game works in pixels with y pointing down, like the screen; metres only exist in this folder.
namespace physics {

    constexpr float PIXELS_PER_METER = 32.0f;
    constexpr float GRAVITY = 18.0f;           // m/s^2; a bit above real life, for snappier play
    constexpr float TIME_STEP = 1.0f / 60.0f;  // fixed, so the simulation doesn't depend on the frame rate

    inline float ToMeters(float px) { 
        return px / PIXELS_PER_METER;
    }

    inline b2Vec2 ToMeters(Vector2 px) { 
        return {px.x / PIXELS_PER_METER, px.y / PIXELS_PER_METER};
    }

    inline Vector2 ToPixels(b2Vec2 m) { 
        return {m.x * PIXELS_PER_METER, m.y * PIXELS_PER_METER};
    }

    enum class BodyType { Static, Dynamic };

    struct Material {
        float density = 1.0f;
        float friction = 0.6f;
        float restitution = 0.05f;
    };

    
    struct Impact {
        const b2Body* a;
        const b2Body* b;
        float impulse;
        Vector2 point;
    };

    class World {
    public:
        World();
        World(const World&) = delete;   
        World& operator=(const World&) = delete;

        // Runs as many fixed steps as the elapsed time needs. Replaces the previous impacts().
        void Update(float dt);

        // The world owns every body it creates
        b2Body* CreateBox(Vector2 centerPx, Vector2 sizePx, BodyType type, const Material& material);
        b2Body* CreateCircle(Vector2 centerPx, float radiusPx, BodyType type, const Material& material);
        void DestroyBody(b2Body* body);

        const std::vector<Impact>& impacts() const { return impacts_; }
        bool allAsleep() const;   
        
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
