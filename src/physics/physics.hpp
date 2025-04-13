#pragma once

#include <set>
#include <memory>

#include "scene/system.hpp"
#include "math/vector3.hpp"

class btCollisionObject;
class btCollisionConfiguration;
class btBroadphaseInterface;
class btCollisionDispatcher;
class btConstraintSolver;
class btDiscreteDynamicsWorld;

namespace xen {
class Entity;
class CollisionObject;

using CollisionPair = std::pair<btCollisionObject const*, btCollisionObject const*>;
using CollisionPairs = std::set<CollisionPair>;

class XEN_API Raycast {
private:
    bool hit;
    Vector3f point_world;
    CollisionObject* collision_object;

public:
    Raycast(bool hit, Vector3f const& point_world, CollisionObject* collision_object) :
        hit(hit), point_world(point_world), collision_object(collision_object)
    {
    }

    [[nodiscard]] bool has_hit() const { return hit; }
    [[nodiscard]] Vector3f const& get_point_world() const { return point_world; }
    [[nodiscard]] CollisionObject* get_collision_object() const { return collision_object; }
};

class XEN_API Physics : public System {
private:
    std::unique_ptr<btCollisionConfiguration> collision_configuration;
    std::unique_ptr<btBroadphaseInterface> broadphase;
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btConstraintSolver> solver;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamics_world;
    CollisionPairs pairs_last_update;

    Vector3f gravity = Vector3f(0.0f, -9.81f, 0.0f);
    float air_density = 1.2f;

public:
    Physics();
    ~Physics() override;

    void start() override;
    void update() override;

    [[nodiscard]] Raycast raytest(Vector3f const& start, Vector3f const& end) const;

    [[nodiscard]] Vector3f const& get_gravity() const { return gravity; }
    void set_gravity(Vector3f const& gravity);

    [[nodiscard]] float get_air_dencity() const { return air_density; }
    void set_air_dencity(float air_density);

    btBroadphaseInterface* get_broadphase() { return broadphase.get(); }

    btDiscreteDynamicsWorld* get_dynamics_world() { return dynamics_world.get(); }

private:
    void check_for_collision_events();
};
}