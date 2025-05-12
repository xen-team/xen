#pragma once

#include <component.hpp>

namespace xen {
class RigidBody final : public Component {
    friend class PhysicsSystem;

public:
    /// Creates a rigid body with given mass & bounciness.
    /// \param mass Mass of the rigid body. A value less than or equal to 0 represents an infinite mass.
    /// \param bounciness Coefficient of restitution (must be between 0 & 1).
    RigidBody(float mass, float bounciness);

    constexpr float get_mass() const { return mass; }

    constexpr float get_inv_mass() const { return inv_mass; }

    constexpr float get_bounciness() const { return bounciness; }

    constexpr Vector3f const& get_forces() const { return forces; }

    constexpr Vector3f const& get_velocity() const { return velocity; }

    void set_mass(float mass);

    void set_bounciness(float bounciness);

    template <typename... Args>
    constexpr void set_forces(Args const&... forces)
    {
        this->forces = (forces + ...);
    }

    constexpr void set_velocity(Vector3f const& velocity) { this->velocity = velocity; }

private:
    float mass{};     ///< Mass of the rigid body.
    float inv_mass{}; ///< Inverse mass of the rigid body.
    float bounciness{
    }; ///< Coefficient of restitution, determining the amount of energy kept by the rigid body when bouncing off.

    Vector3f forces{};       ///< Additional forces applied to the rigid body; gravity is computed independently later.
    Vector3f velocity{};     ///< Velocity of the rigid body.
    Vector3f old_position{}; ///< Previous position of the rigid body.
};
}