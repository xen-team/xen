#pragma once

#include <cstddef>
#include <math/vector/vector3.hpp>

namespace xen {
class Matrix2;
class Matrix4;

class XEN_API Matrix3 {
public:
    using row_t = Vector3f;
    constexpr static uint8_t Count = 3;

    std::array<row_t, Count> rows{};

    static Matrix3 const Identity;

public:
    constexpr Matrix3(float diagonal = 1.0f);

    constexpr Matrix3(Matrix2 const& src);

    constexpr Matrix3(Matrix4 const& src);

    constexpr Matrix3(
        float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22
    );

    // constexpr Matrix3(std::array<float, static_cast<std::size_t>(Count* Count)> src);

    constexpr Matrix3(std::array<row_t, Count> src);

    constexpr Matrix3(Matrix3 const& src) = default;

    constexpr Matrix3(Matrix3&& src) noexcept = default;

    ~Matrix3() = default;

    [[nodiscard]] constexpr Matrix3 add(Matrix3 const& other) const;

    [[nodiscard]] constexpr Matrix3 subtract(Matrix3 const& other) const;

    [[nodiscard]] constexpr Matrix3 multiply(Matrix3 const& other) const;

    [[nodiscard]] constexpr row_t multiply(row_t const& other) const;

    [[nodiscard]] constexpr Matrix3 divide(Matrix3 const& other) const;

    [[nodiscard]] constexpr row_t transform(row_t const& other) const;

    [[nodiscard]] constexpr Matrix3 scale(row_t const& other) const;

    [[nodiscard]] constexpr Matrix3 negate() const;

    [[nodiscard]] constexpr Matrix3 inverse() const;

    [[nodiscard]] constexpr Matrix3 transpose() const;

    [[nodiscard]] constexpr float determinant() const;

    [[nodiscard]] constexpr Matrix2 submatrix(uint32_t row, uint32_t col) const;

    [[nodiscard]] constexpr row_t const& operator[](uint32_t index) const;
    [[nodiscard]] constexpr row_t& operator[](uint32_t index);

    constexpr auto operator<=>(Matrix3 const&) const = default;

    constexpr Matrix3& operator=(Matrix3 const& other) = default;

    constexpr Matrix3& operator=(Matrix3&& other) noexcept = default;

    constexpr bool operator==(Matrix3 const& other) const;
    constexpr bool operator!=(Matrix3 const& other) const;

    constexpr Matrix3 operator-() const;

    friend constexpr Matrix3 operator+(Matrix3 const& lhs, Matrix3 const& rhs);
    friend constexpr Matrix3 operator-(Matrix3 const& lhs, Matrix3 const& rhs);
    friend constexpr Matrix3 operator*(Matrix3 const& lhs, Matrix3 const& rhs);
    friend constexpr Matrix3 operator/(Matrix3 const& lhs, Matrix3 const& rhs);
    friend constexpr Matrix3 operator*(row_t const& lhs, Matrix3 const& rhs);
    friend constexpr Matrix3 operator/(row_t const& lhs, Matrix3 const& rhs);
    friend constexpr Matrix3 operator*(Matrix3 const& lhs, row_t const& rhs);
    friend constexpr Matrix3 operator/(Matrix3 const& lhs, row_t const& rhs);
    friend constexpr Matrix3 operator*(float lhs, Matrix3 const& rhs);
    friend constexpr Matrix3 operator/(float lhs, Matrix3 const& rhs);
    friend constexpr Matrix3 operator*(Matrix3 const& lhs, float rhs);
    friend constexpr Matrix3 operator/(Matrix3 const& lhs, float rhs);

    constexpr Matrix3& operator+=(Matrix3 const& other);
    constexpr Matrix3& operator-=(Matrix3 const& other);
    constexpr Matrix3& operator*=(Matrix3 const& other);
    constexpr Matrix3& operator/=(Matrix3 const& other);
    constexpr Matrix3& operator*=(row_t const& other);
    constexpr Matrix3& operator/=(row_t const& other);
    constexpr Matrix3& operator*=(float other);
    constexpr Matrix3& operator/=(float other);

    constexpr friend std::ostream& operator<<(std::ostream& stream, Matrix3 const& matrix);

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

#include "matrix3.inl"