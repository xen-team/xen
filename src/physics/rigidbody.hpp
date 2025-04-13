#pragma once
#include "collision_object.hpp"

#include "math/vector3.hpp"
#include "scene/component.hpp"

struct btDefaultMotionState;
class btRigidBody;

namespace xen {
class XEN_API Rigidbody : public Component, public CollisionObject {
    inline static bool const registered = register_component<Rigidbody>();

private:
    std::unique_ptr<btRigidBody> rigid_body;

public:
    Rigidbody() = default;
    explicit Rigidbody(
        std::unique_ptr<Collider>&& collider = nullptr, float mass = 1.0f, float friction = 0.2f,
        Vector3f const& linear_factor = Vector3f(1.0f), Vector3f const& angular_factor = Vector3f(1.0f)
    );
    ~Rigidbody();

    void start() override;
    void update() override;

    bool in_frustum(Frustum const& frustum) override;
    void clear_forces();
    void set_mass(float mass) override;
    void set_gravity(Vector3f const& gravity) override;
    void set_linear_factor(Vector3f const& linear_factor) override;
    void set_angular_factor(Vector3f const& angular_factor) override;
    void set_linear_velocity(Vector3f const& linear_velocity) override;
    void set_angular_velocity(Vector3f const& angular_velocity) override;

    template <class Archive>
    void save(Archive& ar) const
    {
        // ar(colliders, mass, friction, friction_rolling, friction_spinning, linear_factor, angular_factor);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        // ar(colliders, mass, friction, friction_rolling, friction_spinning, linear_factor, angular_factor);
    }

protected:
    void recalculate_mass() override;
};
}