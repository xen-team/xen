#pragma once

#include <math/vector/vector2.hpp>

namespace xen {
class Matrix3;
class Matrix4;

class XEN_API Matrix2 {
public:
    using row_t = Vector2f;
    constexpr static uint8_t Count = 2;

    std::array<row_t, Count> rows{};

    static Matrix2 const Identity;

public:
    constexpr Matrix2(float diagonal = 1.0f);

    constexpr Matrix2(Matrix3 const& src);

    constexpr Matrix2(Matrix4 const& src);

    constexpr Matrix2(float x1, float x2, float y1, float y2);

    // constexpr Matrix2(std::array<float, static_cast<std::size_t>(Count* Count)> src);

    constexpr Matrix2(std::array<row_t, Count> src);

    constexpr Matrix2(Matrix2 const& src) = default;

    constexpr Matrix2(Matrix2&& src) noexcept = default;

    ~Matrix2() = default;

    [[nodiscard]] constexpr Matrix2 add(Matrix2 const& other) const;

    [[nodiscard]] constexpr Matrix2 subtract(Matrix2 const& other) const;

    [[nodiscard]] constexpr Matrix2 multiply(Matrix2 const& other) const;

    [[nodiscard]] constexpr Matrix2 divide(Matrix2 const& other) const;

    [[nodiscard]] constexpr row_t transform(row_t const& other) const;

    [[nodiscard]] constexpr Matrix2 scale(row_t const& other) const;

    [[nodiscard]] constexpr Matrix2 negate() const;

    [[nodiscard]] constexpr Matrix2 inverse() const;

    [[nodiscard]] constexpr Matrix2 transpose() const;

    [[nodiscard]] constexpr float determinant() const;

    [[nodiscard]] constexpr float submatrix(uint32_t row, uint32_t col) const;

    [[nodiscard]] constexpr row_t const& operator[](uint32_t index) const;
    [[nodiscard]] constexpr row_t& operator[](uint32_t index);

    constexpr auto operator<=>(Matrix2 const&) const = default;

    constexpr Matrix2& operator=(Matrix2 const& other) = default;

    constexpr Matrix2& operator=(Matrix2&& other) noexcept = default;

    constexpr bool operator==(Matrix2 const& other) const;
    constexpr bool operator!=(Matrix2 const& other) const;

    constexpr Matrix2 operator-() const;

    friend constexpr Matrix2 operator+(Matrix2 const& lhs, Matrix2 const& rhs);
    friend constexpr Matrix2 operator-(Matrix2 const& lhs, Matrix2 const& rhs);
    friend constexpr Matrix2 operator*(Matrix2 const& lhs, Matrix2 const& rhs);
    friend constexpr Matrix2 operator/(Matrix2 const& lhs, Matrix2 const& rhs);
    friend constexpr Matrix2 operator*(Vector2f const& lhs, Matrix2 const& rhs);
    friend constexpr Matrix2 operator/(Vector2f const& lhs, Matrix2 const& rhs);
    friend constexpr Matrix2 operator*(Matrix2 const& lhs, Vector2f const& rhs);
    friend constexpr Matrix2 operator/(Matrix2 const& lhs, Vector2f const& rhs);
    friend constexpr Matrix2 operator*(float lhs, Matrix2 const& rhs);
    friend constexpr Matrix2 operator/(float lhs, Matrix2 const& rhs);
    friend constexpr Matrix2 operator*(Matrix2 const& lhs, float rhs);
    friend constexpr Matrix2 operator/(Matrix2 const& lhs, float rhs);

    constexpr Matrix2& operator+=(Matrix2 const& other);
    constexpr Matrix2& operator-=(Matrix2 const& other);
    constexpr Matrix2& operator*=(Matrix2 const& other);
    constexpr Matrix2& operator/=(Matrix2 const& other);
    constexpr Matrix2& operator*=(Vector2f const& other);
    constexpr Matrix2& operator/=(Vector2f const& other);
    constexpr Matrix2& operator*=(float other);
    constexpr Matrix2& operator/=(float other);

    constexpr friend std::ostream& operator<<(std::ostream& stream, Matrix2 const& matrix);

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

#include "matrix2.inl"