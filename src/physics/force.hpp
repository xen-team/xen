#pragma once

#include "math/vector3.hpp"
#include "utils/time.hpp"

namespace xen {
class XEN_API Force {
private:
    Vector3f force;
    Vector3f position;
    Time time_left;
    bool never_expires;

public:
    explicit Force(Vector3f const& force, Vector3f const& position = {});

    Force(Vector3f const& force, Time const& time, Vector3f const& position = {});

    void update(Time const& delta);

    [[nodiscard]] bool is_expired() const { return !never_expires && time_left.as_microseconds<float>() <= 0.0f; }

    [[nodiscard]] Vector3f const& get_force() const { return force; }
    void set_force(Vector3f const& force) { this->force = force; }

    [[nodiscard]] Time const& get_time_left() const { return time_left; }
    void set_time_left(Time const& time_left) { this->time_left = time_left; }

    [[nodiscard]] Vector3f const& get_position() const { return position; }
    void set_position(Vector3f const& position) { this->position = position; }
};
}