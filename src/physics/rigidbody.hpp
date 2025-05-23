#pragma once
#include "collision_object.hpp"

#include "component.hpp"

struct btDefaultMotionState;
class btRigidBody;

namespace xen {
struct FrameTimeInfo;

class XEN_API Rigidbody : public Component, public CollisionObject {

public:
    explicit Rigidbody(
        std::unique_ptr<Collider>&& collider = nullptr, float mass = 1.0f, float friction = 0.2f,
        Vector3f const& linear_factor = Vector3f(1.0f), Vector3f const& angular_factor = Vector3f(1.0f)
    );
    ~Rigidbody();

    void start(Transform& transform);
    void update(FrameTimeInfo const& time_info, Transform& transform);

    bool in_frustum(Frustum const& frustum) override;
    void clear_forces();

    void set_mass(float new_mass) override;
    void set_gravity(Vector3f const& new_gravity) override;
    void set_linear_factor(Vector3f const& new_linear_factor) override;
    void set_angular_factor(Vector3f const& new_angular_factor) override;
    void set_linear_velocity(Vector3f const& new_linear_velocity) override;
    void set_angular_velocity(Vector3f const& new_angular_velocity) override;

    void set_friction_rolling(float new_friction_rolling);
    void set_friction_spinning(float new_friction_spinning);

    btRigidBody* get_bullet_rigidbody() { return rigid_body.get(); }

private:
    std::unique_ptr<btRigidBody> rigid_body;

protected:
    void recalculate_mass() override;
};
}