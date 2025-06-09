#pragma once

#include <component.hpp>

namespace xen {
class Listener final : public Component {
public:
    Listener() = default;

    explicit Listener(Vector3f const& position) { set_position(position); }

    Listener(Vector3f const& position, Vector3f const& forward_direction, Vector3f const& up_direction);

    Listener(Vector3f const& position, Matrix3 const& rotation);

    Listener(Listener const&) = delete;
    Listener(Listener&&) noexcept = default;

    Listener& operator=(Listener const&) = delete;
    Listener& operator=(Listener&&) noexcept = default;

    /// Sets the listener's gain (master volume).
    /// \param gain Listener's gain; must be positive. 1 is the default.
    void set_gain(float gain) const;

    /// Recovers the listener's gain (master volume).
    /// \return Listener's gain.
    float recover_gain() const;

    void set_position(Vector3f const& position) const;

    Vector3f recover_position() const;

    void set_velocity(Vector3f const& velocity) const;

    Vector3f recover_velocity() const;

    /// Sets the listener's forward orientation. Uses the Y axis as the up direction.
    /// \param forward_direction Forward direction to be set.
    void set_orientation(Vector3f const& forward_direction) const;

    /// Sets the listener's forward & up orientation.
    /// \param forward_direction Forward direction to be set.
    /// \param up_direction Up direction to be set.
    void set_orientation(Vector3f const& forward_direction, Vector3f const& up_direction) const;

    /// Sets the listener's forward & up orientation, respectively taken from the given rotation matrix's 3rd & 2nd
    /// columns.
    /// \note The forward direction is reversed, as the engine uses a right-handed coordinate system with its Z pointing
    /// to the viewer.
    /// \param rotation Rotation matrix from which to extract the orientation.
    void set_orientation(Matrix3 const& rotation) const;

    std::pair<Vector3f, Vector3f> recover_orientation() const;

    Vector3f recover_forward_orientation() const;

    Vector3f recover_up_orientation() const;
};
}