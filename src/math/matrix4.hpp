#pragma once

#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

#include <array>
#include <cstddef>

namespace xen {
class Matrix2;
class Matrix3;

class XEN_API Matrix4 {
public:
    using row_t = Vector4f;
    constexpr static uint8_t COUNT = 4;

    std::array<row_t, 4> rows{};

public:
    constexpr Matrix4(float diagonal = 1.0f);

    constexpr Matrix4(Matrix2 const& src);

    constexpr Matrix4(Matrix3 const& src);

    constexpr explicit Matrix4(std::array<float, static_cast<std::size_t>(COUNT* COUNT)> src);

    constexpr explicit Matrix4(std::array<row_t, COUNT> src);

    constexpr Matrix4(Matrix4 const& src) = default;

    constexpr Matrix4(Matrix4&& src) noexcept = default;

    ~Matrix4() = default;

    [[nodiscard]] constexpr Matrix4 add(Matrix4 const& other) const;

    [[nodiscard]] constexpr Matrix4 subtract(Matrix4 const& other) const;

    [[nodiscard]] constexpr Matrix4 multiply(Matrix4 const& other) const;

    [[nodiscard]] constexpr row_t multiply(row_t const& other) const;

    [[nodiscard]] constexpr Matrix4 divide(Matrix4 const& other) const;

    [[nodiscard]] constexpr row_t transform(row_t const& other) const;

    [[nodiscard]] constexpr Matrix4 translate(Vector2f const& other) const;

    [[nodiscard]] constexpr Matrix4 translate(Vector3f const& other) const;

    [[nodiscard]] constexpr Matrix4 scale(Vector2f const& other) const;

    [[nodiscard]] constexpr Matrix4 scale(Vector3f const& other) const;

    [[nodiscard]] constexpr Matrix4 scale(row_t const& other) const;

    [[nodiscard]] constexpr Matrix4 rotate(float angle, Vector3f const& axis) const;

    [[nodiscard]] constexpr Matrix4 negate() const;

    [[nodiscard]] constexpr Matrix4 inverse() const;

    [[nodiscard]] constexpr Matrix4 transpose() const;

    [[nodiscard]] constexpr float determinant() const;

    [[nodiscard]] constexpr Matrix3 submatrix(uint32_t row, uint32_t col) const;

    constexpr static Matrix4
    tranform_matrix(Vector3f const& translation, Vector3f const& rotation, Vector3f const& scale);

    constexpr static Matrix4 perspective_matrix(float fov, float aspect_ratio, float near, float far);

    constexpr static Matrix4 perspective_matrix(float fov, float aspect_ratio, float near);

    constexpr static Matrix4
    orthographic_matrix(float left, float right, float bottom, float top, float near, float far);

    constexpr static Matrix4 frustum_matrix(float left, float right, float bottom, float top, float near, float far);

    constexpr static Matrix4 view_matrix(Vector3f const& position, Vector3f const& rotation);

    constexpr static Vector3f
    project(Vector3f const& world_space, Matrix4 const& view_matrix, Matrix4 const& projection_matrix);

    constexpr static Vector3f
    unproject(Vector3f const& screen_space, Matrix4 const& view_matrix, Matrix4 const& projection_matrix);

    constexpr static Matrix4 look_at(Vector3f const& eye, Vector3f const& centre, Vector3f const& up = Vector3f::up);

    [[nodiscard]] constexpr row_t const& operator[](uint32_t index) const;
    [[nodiscard]] constexpr row_t& operator[](uint32_t index);

    auto operator<=>(Matrix4 const&) const = default;

    constexpr Matrix4& operator=(Matrix4 const& other) = default;

    constexpr Matrix4& operator=(Matrix4&& other) noexcept = default;

    constexpr bool operator==(Matrix4 const& other) const;
    constexpr bool operator!=(Matrix4 const& other) const;

    constexpr Matrix4 operator-() const;

    friend constexpr Matrix4 operator+(Matrix4 const& lhs, Matrix4 const& rhs);
    friend constexpr Matrix4 operator-(Matrix4 const& lhs, Matrix4 const& rhs);
    friend constexpr Matrix4 operator*(Matrix4 const& lhs, Matrix4 const& rhs);
    friend constexpr Matrix4 operator/(Matrix4 const& lhs, Matrix4 const& rhs);
    friend constexpr Matrix4 operator*(row_t const& lhs, Matrix4 const& rhs);
    friend constexpr Matrix4 operator/(row_t const& lhs, Matrix4 const& rhs);
    friend constexpr Matrix4 operator*(Matrix4 const& lhs, row_t const& rhs);
    friend constexpr Matrix4 operator/(Matrix4 const& lhs, row_t const& rhs);
    friend constexpr Matrix4 operator*(float lhs, Matrix4 const& rhs);
    friend constexpr Matrix4 operator/(float lhs, Matrix4 const& rhs);
    friend constexpr Matrix4 operator*(Matrix4 const& lhs, float rhs);
    friend constexpr Matrix4 operator/(Matrix4 const& lhs, float rhs);

    constexpr Matrix4& operator+=(Matrix4 const& other);
    constexpr Matrix4& operator-=(Matrix4 const& other);
    constexpr Matrix4& operator*=(Matrix4 const& other);
    constexpr Matrix4& operator/=(Matrix4 const& other);
    constexpr Matrix4& operator*=(row_t const& other);
    constexpr Matrix4& operator/=(row_t const& other);
    constexpr Matrix4& operator*=(float other);
    constexpr Matrix4& operator/=(float other);

    constexpr friend std::ostream& operator<<(std::ostream& stream, Matrix4 const& matrix);

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(rows);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(rows);
    }
};
}

#include "matrix4.inl"