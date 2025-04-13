#pragma once
#include "collision_object.hpp"

#include "math/vector3.hpp"
#include "scene/component.hpp"
#include "colliders/collider.hpp"

class btPairCachingGhostObject;
class btKinematicCharacterController;

namespace xen {
class XEN_API KinematicCharacter : public Component, public CollisionObject {
    inline static bool const registered = register_component<KinematicCharacter>();

private:
    Vector3f up;
    float step_height;
    float fall_speed;
    float jump_speed;
    float max_height;
    bool interpolate;

    std::unique_ptr<btPairCachingGhostObject> ghost_object;
    std::unique_ptr<btKinematicCharacterController> controller;

public:
    explicit KinematicCharacter(
        std::unique_ptr<Collider>&& collider = nullptr, float mass = 1.0f, float friction = 0.2f
    );
    ~KinematicCharacter();

    void start() override;
    void update() override;

    bool in_frustum(Frustum const& frustum) override;
    void set_mass(float mass) override;
    void set_gravity(Vector3f const& gravity) override;
    void set_linear_factor(Vector3f const& linear_factor) override;
    void set_angular_factor(Vector3f const& angular_factor) override;
    void set_linear_velocity(Vector3f const& linear_velocity) override;
    void set_angular_velocity(Vector3f const& angular_velocity) override;

    Vector3f const& get_up() const { return up; }
    void set_up(Vector3f const& up);

    float get_step_height() const { return step_height; }
    void set_step_height(float step_height);

    float get_fall_speed() const { return fall_speed; }
    void set_fall_speed(float fall_speed);

    float get_jump_speed() const { return jump_speed; }
    void set_jump_speed(float jump_speed);

    float get_max_jump_height() const { return max_height; }
    void set_max_jump_height(float max_height);

    bool is_interpolate() const { return interpolate; }
    void set_interpolate(bool interpolate);

    bool is_on_ground() const;
    void jump(Vector3f const& direction);
    void set_walk_direction(Vector3f const& direction);

    template <class Archive>
    void save(Archive& ar) const
    {
        // ar(colliders, mass, friction, friction_rolling, friction_spinning, up, step_height, fall_speed, jump_speed,
        //    max_height, interpolate);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        // ar(colliders, mass, friction, friction_rolling, friction_spinning, up, step_height, fall_speed, jump_speed,
        //    max_height, interpolate);
    }

protected:
    void recalculate_mass() override;
};
}