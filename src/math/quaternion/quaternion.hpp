#pragma once

#include <math/matrix/matrix4.hpp>
#include <math/vector/vector3.hpp>

namespace xen {
template <typename T>
struct Radians;

class XEN_API Quaternion {
public:
    using value_type = float;

    static Quaternion const Identity;
    static Quaternion const Zero;
    static Quaternion const One;
    static Quaternion const PositiveInfinity;
    static Quaternion const NegativeInfinity;

    value_type x = 0.f;
    value_type y = 0.f;
    value_type z = 0.f;
    value_type w = 1.f;

public:
    constexpr Quaternion() = default;

    constexpr Quaternion(float x, float y, float z, float w);

    constexpr explicit Quaternion(Vector3f const& euler);

    constexpr Quaternion(Matrix4 const& rotation);

    constexpr Quaternion(Vector3f const& axis_x, Vector3f const& axis_y, Vector3f const& axis_z);

    constexpr Quaternion(Vector3f const& axis, Radians<float> angle);

    [[nodiscard]] constexpr bool is_identity() const;

    [[nodiscard]] constexpr bool is_pure() const;

    [[nodiscard]] constexpr bool is_normalized() const;

    [[nodiscard]] constexpr Vector3f rotation_axis() const;

    [[nodiscard]] constexpr Quaternion inverse() const;

    [[nodiscard]] constexpr Quaternion conjugate() const;

    // [[nodiscard]] constexpr std::pair<Vector3f, float> axis_and_angle() const;

    [[nodiscard]] constexpr Vector3f rotate_point(Vector3f const& point) const;

    [[nodiscard]] constexpr Vector3f rotate_point(Vector3f const& point, Vector3f const& pivot) const;

    [[nodiscard]] static constexpr Quaternion look_at(Vector3f const& forward, Vector3f const& up);

    [[nodiscard]] constexpr Quaternion multiply_inverse(Quaternion const& other) const;

    [[nodiscard]] constexpr float dot(Quaternion const& other) const;

    [[nodiscard]] constexpr Quaternion slerp(Quaternion const& other, float progression) const;

    [[nodiscard]] constexpr Quaternion normalize() const;

    [[nodiscard]] constexpr float length() const;

    [[nodiscard]] constexpr float length_squared() const;

    [[nodiscard]] constexpr float max_component() const;

    [[nodiscard]] constexpr float min_component() const;

    [[nodiscard]] constexpr Matrix4 to_matrix() const;

    [[nodiscard]] constexpr Matrix4 to_rotation_matrix() const;

    [[nodiscard]] static constexpr Quaternion from_rotation_matrix(Matrix4 const& rotation_matrix);

    [[nodiscard]] constexpr Vector3f to_euler() const;

    [[nodiscard]] constexpr float operator[](uint32_t index) const;
    [[nodiscard]] constexpr float& operator[](uint32_t index);

    constexpr bool operator==(Quaternion const& rhs) const;
    constexpr bool operator!=(Quaternion const& rhs) const;

    constexpr Quaternion operator-() const;

    friend constexpr Quaternion operator+(Quaternion const& lhs, Quaternion const& rhs);
    friend constexpr Quaternion operator-(Quaternion const& lhs, Quaternion const& rhs);
    friend constexpr Quaternion operator*(Quaternion const& lhs, Quaternion const& rhs);
    friend constexpr Vector3f operator*(Vector3f const& lhs, Quaternion const& rhs);
    friend constexpr Vector3f operator*(Quaternion const& lhs, Vector3f const& rhs);
    friend constexpr Quaternion operator*(float lhs, Quaternion const& rhs);
    friend constexpr Quaternion operator*(Quaternion const& lhs, float rhs);
    friend constexpr Quaternion operator/(Quaternion const& lhs, float rhs);

    constexpr Quaternion& operator*=(Quaternion const& rhs);
    constexpr Quaternion& operator*=(float rhs);
    constexpr Quaternion& operator*=(Vector3f const& rhs);

    constexpr friend std::ostream& operator<<(std::ostream& stream, Quaternion const& quaternion);

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(x, y, z, w);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(x, y, z, w);
    }
};
}

#include "quaternion.inl"