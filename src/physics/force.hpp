#pragma once

#include "utils/time.hpp"

namespace xen {
class XEN_API Force {

public:
    explicit Force(Vector3f const& force, Vector3f const& position = {});

    Force(Vector3f const& force, float const& time, Vector3f const& position = {});

    void update(float const& delta);

    [[nodiscard]] bool is_expired() const { return !never_expires && time_left <= 0.0f; }

    [[nodiscard]] Vector3f const& get_force() const { return force; }
    void set_force(Vector3f const& force) { this->force = force; }

    [[nodiscard]] float const& get_time_left() const { return time_left; }
    void set_time_left(float const& time_left) { this->time_left = time_left; }

    [[nodiscard]] Vector3f const& get_position() const { return position; }
    void set_position(Vector3f const& position) { this->position = position; }

private:
    Vector3f force;
    Vector3f position;
    float time_left; // in seconds
    bool never_expires;
};
}