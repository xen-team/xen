#pragma once

#include "matrix3.hpp"

#include "matrix2.hpp"
#include "matrix4.hpp"

namespace xen {
constexpr Matrix3::Matrix3(float diagonal)
{
    rows[0][0] = diagonal;
    rows[1][1] = diagonal;
    rows[2][2] = diagonal;
}

constexpr Matrix3::Matrix3(Matrix2 const& src) : rows{src[0], src[1], Vector3f{0.f, 0.f, 1.f}} {}

constexpr Matrix3::Matrix3(Matrix4 const& src) : rows{src[0], src[1], src[2]} {}

constexpr Matrix3::Matrix3(
    float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22
) : rows{{{m00, m10, m20}, {m01, m11, m21}, {m02, m12, m22}}} {};

// constexpr Matrix3::Matrix3(std::array<float, static_cast<std::size_t>(Count* Count)> src) :
// rows{{{src[0], src[3], src[6]}, {src[1], src[4], src[7]}, {src[2], src[5], src[8]}}} {};

constexpr Matrix3::Matrix3(std::array<row_t, Count> src) : rows{src[0], src[1], src[2]} {};

constexpr Matrix3 Matrix3::add(Matrix3 const& other) const
{
    Matrix3 result;

    for (uint32_t row = 0; row < Count; row++) {
        for (uint32_t col = 0; col < Count; col++) {
            result[row][col] = rows[row][col] + other[row][col];
        }
    }

    return result;
}

constexpr Matrix3 Matrix3::subtract(Matrix3 const& other) const
{
    Matrix3 result;

    for (uint32_t row = 0; row < Count; row++) {
        for (uint32_t col = 0; col < Count; col++) {
            result[row][col] = rows[row][col] - other[row][col];
        }
    }

    return result;
}

constexpr Matrix3 Matrix3::multiply(Matrix3 const& other) const
{
    Matrix3 result;

    for (uint32_t row = 0; row < Count; row++) {
        for (uint32_t col = 0; col < Count; col++) {
            result[row][col] =
                rows[0][col] * other[row][0] + rows[1][col] * other[row][1] + rows[2][col] * other[row][2];
        }
    }

    return result;
}

constexpr Matrix3::row_t Matrix3::multiply(row_t const& other) const
{
    row_t result;

    for (uint32_t row = 0; row < Count; row++) {
        result[row] = rows[row][0] * other[0] + rows[row][1] * other[1] + rows[row][2] * other[2];
    }

    return result;
}

constexpr Matrix3 Matrix3::divide(Matrix3 const& other) const
{
    Matrix3 result;

    for (uint32_t row = 0; row < Count; row++) {
        for (uint32_t col = 0; col < Count; col++) {
            result[row][col] =
                rows[0][col] / other[row][0] + rows[1][col] / other[row][1] + rows[2][col] / other[row][2];
        }
    }

    return result;
}

constexpr Matrix3::row_t Matrix3::transform(row_t const& other) const
{
    row_t result;

    for (uint32_t row = 0; row < 3; row++) {
        result[row] = (rows[0][row] * other.x) + (rows[1][row] * other.y) + (rows[2][row] * other.z);
    }

    return result;
}

constexpr Matrix3 Matrix3::scale(row_t const& other) const
{
    Matrix3 result;

    for (uint32_t row = 0; row < Count; row++) {
        for (uint32_t col = 0; col < Count; col++) {
            result[row][col] = rows[row][col] * other[row];
        }
    }

    return result;
}

constexpr Matrix3 Matrix3::negate() const
{
    Matrix3 result;

    for (uint32_t row = 0; row < Count; row++) {
        for (uint32_t col = 0; col < Count; col++) {
            result[row][col] = -rows[row][col];
        }
    }

    return result;
}

constexpr Matrix3 Matrix3::inverse() const
{
    Matrix3 result;

    auto det = determinant();
    if (det == 0.0f) {
        throw std::runtime_error("Can't invert a matrix with a determinant of zero");
    }

    for (uint32_t j = 0; j < Count; j++) {
        for (uint32_t i = 0; i < Count; i++) {
            // Get minor of element [j][i] - not [i][j], this is where the transpose happens.
            auto const minor_submatrix = submatrix(j, i);
            auto const minor = minor_submatrix.determinant();

            // Multiply by (−1)^{i+j}.
            auto const factor = ((i + j) % 2 == 1) ? -1.0f : 1.0f;
            auto const cofactor = minor * factor;

            result[i][j] = cofactor / det;
        }
    }

    return result;
}

constexpr Matrix3 Matrix3::transpose() const
{
    Matrix3 result;

    for (uint32_t row = 0; row < Count; row++) {
        for (uint32_t col = 0; col < Count; col++) {
            result[row][col] = rows[col][row];
        }
    }

    return result;
}

constexpr float Matrix3::determinant() const
{
    float result = 0.0f;

    for (uint32_t i = 0; i < Count; i++) {
        // Get minor of element [0][i].
        auto const minor_submatrix = submatrix(0, i);
        auto const minor = minor_submatrix.determinant();

        // If this is an odd-numbered row, negate the value.
        auto const factor = (i % 2 == 1) ? -1.0f : 1.0f;

        result += factor * rows[0][i] * minor;
    }

    return result;
}

constexpr Matrix2 Matrix3::submatrix(uint32_t row, uint32_t col) const
{
    Matrix2 result;
    uint32_t col_count = 0;
    uint32_t row_count = 0;

    for (uint32_t i = 0; i < Count; i++) {
        if (i != row) {
            col_count = 0;

            for (uint32_t j = 0; j < Count; j++) {
                if (j != col) {
                    result[row_count][col_count] = rows[i][j];
                    col_count++;
                }
            }

            row_count++;
        }
    }

    return result;
}

constexpr Vector3f Matrix3::xyz() const
{
    return Vector3f(rows[0][0], rows[0][1], rows[0][2]);
}

constexpr Matrix3::row_t const& Matrix3::operator[](uint32_t index) const
{
    assert(index < Count);
    return rows[index];
}

constexpr Matrix3::row_t& Matrix3::operator[](uint32_t index)
{
    assert(index < Count);
    return rows[index];
}

constexpr bool Matrix3::operator==(Matrix3 const& other) const
{
    return rows[0] == other.rows[0] && rows[1] == other.rows[1] && rows[2] == other[2];
}

constexpr bool Matrix3::operator!=(Matrix3 const& other) const
{
    return !operator==(other);
}

constexpr Matrix3 Matrix3::operator-() const
{
    return negate();
}

constexpr Matrix3 operator+(Matrix3 const& lhs, Matrix3 const& rhs)
{
    return lhs.add(rhs);
}

constexpr Matrix3 operator-(Matrix3 const& lhs, Matrix3 const& rhs)
{
    return lhs.subtract(rhs);
}

constexpr Matrix3 operator*(Matrix3 const& lhs, Matrix3 const& rhs)
{
    return lhs.multiply(rhs);
}

constexpr Matrix3 operator/(Matrix3 const& lhs, Matrix3 const& rhs)
{
    return lhs.divide(rhs);
}

constexpr Matrix3 operator*(Matrix3::row_t const& lhs, Matrix3 const& rhs)
{
    return rhs.scale(lhs);
}

constexpr Matrix3 operator/(Matrix3::row_t const& lhs, Matrix3 const& rhs)
{
    return rhs.scale(1.0f / lhs);
}

constexpr Matrix3 operator*(Matrix3 const& lhs, Matrix3::row_t const& rhs)
{
    return lhs.scale(rhs);
}

constexpr Matrix3 operator/(Matrix3 const& lhs, Matrix3::row_t const& rhs)
{
    return lhs.scale(1.0f / rhs);
}

constexpr Matrix3 operator*(float lhs, Matrix3 const& rhs)
{
    return rhs.scale(Vector2(lhs));
}

constexpr Matrix3 operator/(float lhs, Matrix3 const& rhs)
{
    return rhs.scale(1.0f / Vector2(lhs));
}

constexpr Matrix3 operator*(Matrix3 const& lhs, float rhs)
{
    return lhs.scale(Vector2(rhs));
}

constexpr Matrix3 operator/(Matrix3 const& lhs, float rhs)
{
    return lhs.scale(1.0f / Vector2(rhs));
}

constexpr Matrix3& Matrix3::operator+=(Matrix3 const& other)
{
    return *this = add(other);
}

constexpr Matrix3& Matrix3::operator-=(Matrix3 const& other)
{
    return *this = subtract(other);
}

constexpr Matrix3& Matrix3::operator*=(Matrix3 const& other)
{
    return *this = multiply(other);
}

constexpr Matrix3& Matrix3::operator/=(Matrix3 const& other)
{
    return *this = divide(other);
}

constexpr Matrix3& Matrix3::operator*=(row_t const& other)
{
    return *this = scale(other);
}

constexpr Matrix3& Matrix3::operator/=(row_t const& other)
{
    return *this = scale(1.0f / other);
}

constexpr Matrix3& Matrix3::operator*=(float other)
{
    return *this = scale(Vector2(other));
}

constexpr Matrix3& Matrix3::operator/=(float other)
{
    return *this = scale(1.0f / Vector2(other));
}

constexpr std::ostream& operator<<(std::ostream& stream, Matrix3 const& matrix)
{
    return stream << matrix[0] << ", " << matrix[1] << ", " << matrix[2];
}
}