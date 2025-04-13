#pragma once
#include "matrix2.hpp"

#include <cassert>
#include <cstddef>
#include <cstring>

#include "matrix3.hpp"
#include "matrix4.hpp"

namespace xen {
constexpr Matrix2::Matrix2(float diagonal)
{
    rows[0][0] = diagonal;
    rows[1][1] = diagonal;
}

constexpr Matrix2::Matrix2(Matrix3 const& src) : rows{src[0], src[1]} {}

constexpr Matrix2::Matrix2(Matrix4 const& src) : rows{src[0], src[1]} {}

constexpr Matrix2::Matrix2(float x1, float x2, float y1, float y2) : rows{{{x1, x2}, {y1, y2}}} {};

constexpr Matrix2::Matrix2(std::array<float, static_cast<std::size_t>(COUNT* COUNT)> src) :
    rows{{{src[0], src[1]}, {src[2], src[3]}}} {};

constexpr Matrix2::Matrix2(std::array<row_t, COUNT> src) : rows{src[0], src[1]} {};

constexpr Matrix2 Matrix2::add(Matrix2 const& other) const
{
    Matrix2 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[row][col] + other[row][col];
        }
    }

    return result;
}

constexpr Matrix2 Matrix2::subtract(Matrix2 const& other) const
{
    Matrix2 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[row][col] - other[row][col];
        }
    }

    return result;
}

constexpr Matrix2 Matrix2::multiply(Matrix2 const& other) const
{
    Matrix2 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[0][col] * other[row][0] + rows[1][col] * other[row][1];
        }
    }

    return result;
}

constexpr Matrix2 Matrix2::divide(Matrix2 const& other) const
{
    Matrix2 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[0][col] / other[row][0] + rows[1][col] / other[row][1];
        }
    }

    return result;
}

constexpr Matrix2::row_t Matrix2::transform(row_t const& other) const
{
    row_t result;

    for (uint32_t row = 0; row < 3; row++) {
        result[row] = rows[0][row] * other.x + rows[1][row] * other.y;
    }

    return result;
}

constexpr Matrix2 Matrix2::scale(row_t const& other) const
{
    Matrix2 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[row][col] * other[row];
        }
    }

    return result;
}

constexpr Matrix2 Matrix2::negate() const
{
    Matrix2 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = -rows[row][col];
        }
    }

    return result;
}

constexpr Matrix2 Matrix2::inverse() const
{
    Matrix2 result;

    auto det = determinant();

    if (det == 0.0f) {
        throw std::runtime_error("Can't invert a matrix with a determinant of zero");
    }

    for (uint32_t j = 0; j < COUNT; j++) {
        for (uint32_t i = 0; i < COUNT; i++) {
            // Get minor of element [j][i] - not [i][j], this is where the transpose happens.
            auto const minor = submatrix(j, i);

            // Multiply by (−1)^{i+j}.
            auto const factor = ((i + j) % 2 == 1) ? -1.0f : 1.0f;
            auto const cofactor = minor * factor;

            result[i][j] = cofactor / det;
        }
    }

    return result;
}

constexpr Matrix2 Matrix2::transpose() const
{
    Matrix2 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[col][row];
        }
    }

    return result;
}

constexpr float Matrix2::determinant() const
{
    float result = 0.0f;

    for (uint32_t i = 0; i < COUNT; i++) {
        // Get minor of element [0][i].
        auto const minor = submatrix(0, i);

        // If this is an odd-numbered row, negate the value.
        auto const factor = (i % 2 == 1) ? -1.0f : 1.0f;

        result += factor * rows[0][i] * minor;
    }

    return result;
}

constexpr float Matrix2::submatrix(uint32_t row, uint32_t col) const
{
    float result = 0.0f;

    for (uint32_t i = 0; i < COUNT; i++) {
        if (i != row) {
            for (uint32_t j = 0; j < COUNT; j++) {
                if (j != col) {
                    result = rows[i][j];
                }
            }
        }
    }

    return result;
}

constexpr Matrix2::row_t const& Matrix2::operator[](uint32_t index) const
{
    assert(index < COUNT);
    return rows[index];
}

constexpr Matrix2::row_t& Matrix2::operator[](uint32_t index)
{
    assert(index < COUNT);
    return rows[index];
}

constexpr bool Matrix2::operator==(Matrix2 const& other) const
{
    return rows[0] == other.rows[0] && rows[1] == other.rows[1];
}

constexpr bool Matrix2::operator!=(Matrix2 const& other) const
{
    return !operator==(other);
}

constexpr Matrix2 Matrix2::operator-() const
{
    return negate();
}

constexpr Matrix2 operator+(Matrix2 const& lhs, Matrix2 const& rhs)
{
    return lhs.add(rhs);
}

constexpr Matrix2 operator-(Matrix2 const& lhs, Matrix2 const& rhs)
{
    return lhs.subtract(rhs);
}

constexpr Matrix2 operator*(Matrix2 const& lhs, Matrix2 const& rhs)
{
    return lhs.multiply(rhs);
}

constexpr Matrix2 operator/(Matrix2 const& lhs, Matrix2 const& rhs)
{
    return lhs.divide(rhs);
}

constexpr Matrix2 operator*(Matrix2::row_t const& lhs, Matrix2 const& rhs)
{
    return rhs.scale(lhs);
}

constexpr Matrix2 operator/(Matrix2::row_t const& lhs, Matrix2 const& rhs)
{
    return rhs.scale(1.0f / lhs);
}

constexpr Matrix2 operator*(Matrix2 const& lhs, Matrix2::row_t const& rhs)
{
    return lhs.scale(rhs);
}

constexpr Matrix2 operator/(Matrix2 const& lhs, Matrix2::row_t const& rhs)
{
    return lhs.scale(1.0f / rhs);
}

constexpr Matrix2 operator*(float lhs, Matrix2 const& rhs)
{
    return rhs.scale(Vector2(lhs));
}

constexpr Matrix2 operator/(float lhs, Matrix2 const& rhs)
{
    return rhs.scale(1.0f / Vector2(lhs));
}

constexpr Matrix2 operator*(Matrix2 const& lhs, float rhs)
{
    return lhs.scale(Vector2(rhs));
}

constexpr Matrix2 operator/(Matrix2 const& lhs, float rhs)
{
    return lhs.scale(1.0f / Vector2(rhs));
}

constexpr Matrix2& Matrix2::operator+=(Matrix2 const& other)
{
    return *this = add(other);
}

constexpr Matrix2& Matrix2::operator-=(Matrix2 const& other)
{
    return *this = subtract(other);
}

constexpr Matrix2& Matrix2::operator*=(Matrix2 const& other)
{
    return *this = multiply(other);
}

constexpr Matrix2& Matrix2::operator/=(Matrix2 const& other)
{
    return *this = divide(other);
}

constexpr Matrix2& Matrix2::operator*=(row_t const& other)
{
    return *this = scale(other);
}

constexpr Matrix2& Matrix2::operator/=(row_t const& other)
{
    return *this = scale(1.0f / other);
}

constexpr Matrix2& Matrix2::operator*=(float other)
{
    return *this = scale(Vector2(other));
}

constexpr Matrix2& Matrix2::operator/=(float other)
{
    return *this = scale(1.0f / Vector2(other));
}

constexpr std::ostream& operator<<(std::ostream& stream, Matrix2 const& matrix)
{
    return stream << matrix[0] << ", " << matrix[1];
}
}