#pragma once
#include "collision_object.hpp"

#include <component.hpp>
#include "colliders/collider.hpp"

class btPairCachingGhostObject;
class btKinematicCharacterController;

namespace xen {
struct FrameTimeInfo;

class XEN_API KinematicCharacter : public Component, public CollisionObject {
private:
    Transform entity_transform;
    Vector3f up;
    float steheight;
    float fall_speed;
    float jump_speed;
    float max_height;
    bool interpolate;
    float capsule_height = 1.f;

    std::unique_ptr<btPairCachingGhostObject> ghost_object;
    std::unique_ptr<btKinematicCharacterController> controller;

public:
    explicit KinematicCharacter(
        std::unique_ptr<Collider>&& collider = nullptr, float mass = 1.0f, float friction = 0.2f
    );
    ~KinematicCharacter();

    void start(Transform& transform);
    void update(FrameTimeInfo const& time_info, Transform& transform);

    bool in_frustum(Frustum const& frustum) override;
    void set_mass(float mass) override;
    void set_gravity(Vector3f const& gravity) override;
    void set_linear_factor(Vector3f const& linear_factor) override;
    void set_angular_factor(Vector3f const& angular_factor) override;
    void set_linear_velocity(Vector3f const& linear_velocity) override;
    void set_angular_velocity(Vector3f const& angular_velocity) override;
    void set_collision_check(bool value);

    Vector3f const& get_up() const { return up; }
    void set_up(Vector3f const& up);

    float get_steheight() const { return steheight; }
    void set_steheight(float steheight);

    float get_fall_speed() const { return fall_speed; }
    void set_fall_speed(float fall_speed);

    float get_jump_speed() const { return jump_speed; }
    void set_jump_speed(float jump_speed);

    float get_max_jumheight() const { return max_height; }
    void set_max_jumheight(float max_height);

    bool is_interpolate() const { return interpolate; }
    void set_interpolate(bool interpolate);

    bool is_on_ground() const;
    void jump(Vector3f const& direction);
    void set_walk_direction(Vector3f const& direction);

protected:
    void recalculate_mass() override;
};
}