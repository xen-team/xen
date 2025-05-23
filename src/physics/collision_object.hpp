#pragma once

#include "force.hpp"

#include "colliders/collider.hpp"
#include "utils/classes.hpp"

class btTransform;
class btCollisionShape;
class btCollisionObject;

namespace xen {
class Frustum;
class XEN_API CollisionObject : NonCopyable {
public:
    CollisionObject(
        std::vector<std::unique_ptr<Collider>>&& colliders = {}, float mass = 1.0f, float friction = 0.2f,
        Vector3f const& linear_factor = Vector3f(1.0f), Vector3f const& angular_factor = Vector3f(1.0f)
    );
    virtual ~CollisionObject() = default;

    virtual bool in_frustum(Frustum const& frustum) = 0;

    Collider* add_collider(std::unique_ptr<Collider>&& collider);
    void remove_collider(Collider* collider);

    Force* add_force(std::unique_ptr<Force>&& force);
    // virtual void clear_forces() = 0;

    bool is_shape_created() const { return shape != nullptr; }

    // void SetChildTransform(Collider* child, Transform const& transform);
    // void AddChild(Collider* child);
    // void RemoveChild(Collider* child);

    void set_ignore_collision_check(CollisionObject* other, bool ignore);

    std::vector<std::unique_ptr<Collider>> const& get_colliders() const { return colliders; }

    std::unique_ptr<Collider> const& get_first_collider() const { return colliders[0]; }

    float get_mass() const { return mass; }

    virtual void set_mass(float mass) = 0;

    Vector3f const& get_gravity() const { return gravity; }

    virtual void set_gravity(Vector3f const& gravity) = 0;

    Vector3f const& get_linear_factor() const { return linear_factor; }

    virtual void set_linear_factor(Vector3f const& linear_factor) = 0;

    Vector3f const& get_angular_factor() const { return angular_factor; }

    virtual void set_angular_factor(Vector3f const& angular_factor) = 0;

    float get_friction() const { return friction; }

    void set_friction(float friction);

    float get_friction_rolling() const { return friction_rolling; }

    void set_friction_rolling(float friction_rolling);

    float get_friction_spinning() const { return friction_spinning; }

    void set_friction_spinning(float friction_spinning);

    Vector3f const& get_linear_velocity() const { return linear_velocity; }

    virtual void set_linear_velocity(Vector3f const& linear_velocity) = 0;

    Vector3f const& get_angular_velocity() const { return angular_velocity; }

    virtual void set_angular_velocity(Vector3f const& angular_velocity) = 0;

protected:
    std::vector<std::unique_ptr<Collider>> colliders;

    float mass;
    Vector3f gravity;

    float friction, friction_rolling, friction_spinning;

    Vector3f linear_factor, angular_factor;

    Vector3f linear_velocity, angular_velocity;

    std::unique_ptr<btCollisionShape> shape;
    btCollisionObject* body = nullptr;

    std::vector<std::unique_ptr<Force>> forces;

protected:
    virtual void recalculate_mass() = 0;

    void create_shape(bool force_single = false);
};
}