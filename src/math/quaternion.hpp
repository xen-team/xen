#pragma once

#include "matrix4.hpp"
#include "vector3.hpp"

namespace xen {
class XEN_API Quaternion {
public:
    using value_type = float;

    static Quaternion const zero;
    static Quaternion const one;
    static Quaternion const positive_infinity;
    static Quaternion const negative_infinity;

    value_type x = 0.f, y = 0.f, z = 0.f, w = 1.f;

public:
    constexpr Quaternion() = default;

    constexpr Quaternion(float x, float y, float z, float w);

    constexpr explicit Quaternion(Vector3f const& source);

    constexpr Quaternion(Matrix4 const& source);

    constexpr Quaternion(Vector3f const& axis_x, Vector3f const& axis_y, Vector3f const& axis_z);

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

    [[nodiscard]] constexpr static Quaternion from_rotation_matrix(Matrix4 const& rotation_matrix);

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

    constexpr Quaternion& operator*=(Quaternion const& rhs);
    constexpr Quaternion& operator*=(float rhs);

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