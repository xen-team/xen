#pragma once

#include <system.hpp>

namespace xen {
class PhysicsSystem final : public System {
public:
    PhysicsSystem();

    constexpr Vector3f const& get_gravity() const { return gravity; }

    constexpr float get_friction() const { return friction; }

    void set_gravity(Vector3f const& gravity) { this->gravity = gravity; }

    void set_friction(float friction)
    {
        Log::rt_assert(friction >= 0.f && friction <= 1.f, "Error: Friction coefficient must be between 0 & 1.");
        this->friction = friction;
    }

    bool update(FrameTimeInfo const& time_info) override;

private:
    Vector3f gravity = Vector3f(0.f, -9.80665f, 0.f); ///< Gravity acceleration.
    float friction = 0.95f;

private:
    void solve_constraints();
};
}