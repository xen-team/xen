#pragma once

#include "matrix4.hpp"

#include <cassert>
#include <cstring>

#include "matrix2.hpp"
#include "matrix3.hpp"

namespace xen {
constexpr Matrix4::Matrix4(float diagonal)
{
    rows[0][0] = diagonal;
    rows[1][1] = diagonal;
    rows[2][2] = diagonal;
    rows[3][3] = diagonal;
}

constexpr Matrix4::Matrix4(Matrix2 const& src) :
    rows{src[0], src[1], Vector4f{0.f, 0.f, 1.f, 0.f}, Vector4f{0.f, 0.f, 0.f, 1.f}}
{
}

constexpr Matrix4::Matrix4(Matrix3 const& src) : rows{src[0], src[1], src[2], Vector4f{0.f, 0.f, 0.f, 1.f}} {}

constexpr Matrix4::Matrix4(std::array<float, static_cast<std::size_t>(COUNT* COUNT)> src) :
    rows{{{src[0], src[1], src[2]}, {src[3], src[4], src[5]}, {src[6], src[7], src[8]}}} {};

constexpr Matrix4::Matrix4(std::array<row_t, COUNT> src) : rows{src[0], src[1], src[2], src[3]} {};

constexpr Matrix4 Matrix4::add(Matrix4 const& other) const
{
    Matrix4 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[row][col] + other[row][col];
        }
    }

    return result;
}

constexpr Matrix4 Matrix4::subtract(Matrix4 const& other) const
{
    Matrix4 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[row][col] - other[row][col];
        }
    }

    return result;
}

constexpr Matrix4 Matrix4::multiply(Matrix4 const& other) const
{
    Matrix4 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = result[row][col] = rows[0][col] * other[row][0] + rows[1][col] * other[row][1] +
                                                  rows[2][col] * other[row][2] + rows[3][col] * other[row][3];
        }
    }

    return result;
}

constexpr Matrix4::row_t Matrix4::multiply(row_t const& other) const
{
    row_t result;

    for (uint32_t row = 0; row < COUNT; row++) {
        result[row] = rows[0][row] * other.x + rows[1][row] * other.y + rows[2][row] * other.z + rows[3][row] * other.w;
    }

    return result;
}

constexpr Matrix4 Matrix4::divide(Matrix4 const& other) const
{
    Matrix4 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = result[row][col] = rows[0][col] / other[row][0] + rows[1][col] / other[row][1] +
                                                  rows[2][col] / other[row][2] + rows[3][col] / other[row][3];
        }
    }

    return result;
}

constexpr Matrix4::row_t Matrix4::transform(row_t const& other) const
{
    row_t result;

    for (uint32_t row = 0; row < COUNT; row++) {
        result[row] = rows[0][row] * other.x + rows[1][row] * other.y + rows[2][row] * other.z + rows[3][row] * other.w;
    }

    return result;
}

constexpr Matrix4 Matrix4::translate(Vector2f const& other) const
{
    Matrix4 result(*this);

    for (uint32_t col = 0; col < COUNT; col++) {
        result[3][col] += rows[0][col] * other.x + rows[1][col] * other.y;
    }

    return result;
}

constexpr Matrix4 Matrix4::translate(Vector3f const& other) const
{
    Matrix4 result(*this);

    for (uint32_t col = 0; col < COUNT; col++) {
        result[3][col] += rows[0][col] * other.x + rows[1][col] * other.y + rows[2][col] * other.z;
    }

    return result;
}

constexpr Matrix4 Matrix4::scale(Vector2f const& other) const
{
    Matrix4 result;

    for (uint32_t row = 0; row < 2; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[row][col] * other[row];
        }
    }

    result[2] = rows[2];
    result[3] = rows[3];
    return result;
}

constexpr Matrix4 Matrix4::scale(Vector3f const& other) const
{
    Matrix4 result;

    for (uint32_t row = 0; row < 3; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[row][col] * other[row];
        }
    }

    result[3] = rows[3];
    return result;
}

constexpr Matrix4 Matrix4::scale(row_t const& other) const
{
    Matrix4 result(*this);

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] *= other[row];
        }
    }

    return result;
}

constexpr Matrix4 Matrix4::rotate(float angle, Vector3f const& axis) const
{
    Matrix4 result;

    auto const c = std::cos(angle);
    auto const s = std::sin(angle);
    auto const o = 1.0f - c;
    auto const xy = axis.x * axis.y;
    auto const yz = axis.y * axis.z;
    auto const xz = axis.x * axis.z;
    auto const xs = axis.x * s;
    auto const ys = axis.y * s;
    auto const zs = axis.z * s;

    Matrix3 f;
    f[0][0] = axis.x * axis.x * o + c;
    f[0][1] = xy * o + zs;
    f[0][2] = xz * o - ys;
    f[1][0] = xy * o - zs;
    f[1][1] = axis.y * axis.y * o + c;
    f[1][2] = yz * o + xs;
    f[2][0] = xz * o + ys;
    f[2][1] = yz * o - xs;
    f[2][2] = axis.z * axis.z * o + c;

    for (uint32_t row = 0; row < 3; row++) {
        for (uint32_t col = 0; col < 4; col++) {
            result[row][col] = rows[0][col] * f[row][0] + rows[1][col] * f[row][1] + rows[2][col] * f[row][2];
        }
    }

    result[3] = rows[3];
    return result;
}

constexpr Matrix4 Matrix4::negate() const
{
    Matrix4 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = -rows[row][col];
        }
    }

    return result;
}

constexpr Matrix4 Matrix4::inverse() const
{
    Matrix4 result;

    auto det = determinant();

    if (std::abs(det) < std::numeric_limits<float>::epsilon()) {
        return Matrix4(0.f);
        // throw std::runtime_error("Can't invert a matrix with a determinant close to zero");
    }

    for (uint32_t j = 0; j < COUNT; j++) {
        for (uint32_t i = 0; i < COUNT; i++) {
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

constexpr Matrix4 Matrix4::transpose() const
{
    Matrix4 result;

    for (uint32_t row = 0; row < COUNT; row++) {
        for (uint32_t col = 0; col < COUNT; col++) {
            result[row][col] = rows[col][row];
        }
    }

    return result;
}

constexpr float Matrix4::determinant() const
{
    float result = 0.0f;

    for (uint32_t i = 0; i < COUNT; i++) {
        // Get minor of element [0][i].
        auto const minor_submatrix = submatrix(0, i);
        auto const minor = minor_submatrix.determinant();

        // If this is an odd-numbered row, negate the value.
        auto const factor = (i % 2 == 1) ? -1.0f : 1.0f;

        result += factor * rows[0][i] * minor;
    }

    return result;
}

constexpr Matrix3 Matrix4::submatrix(uint32_t row, uint32_t col) const
{
    Matrix3 result;
    uint32_t col_count = 0;
    uint32_t row_count = 0;

    for (uint32_t i = 0; i < COUNT; i++) {
        if (i != row) {
            col_count = 0;

            for (uint32_t j = 0; j < COUNT; j++) {
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

constexpr Matrix4 Matrix4::tranform_matrix(Vector3f const& translation, Vector3f const& rotation, Vector3f const& scale)
{
    Matrix4 result;
    result = result.translate(translation);
    result = result.rotate(rotation.x, Vector3f::right);
    result = result.rotate(rotation.y, Vector3f::up);
    result = result.rotate(rotation.z, Vector3f::front);
    result = result.scale(scale);
    return result;
}

constexpr Matrix4 Matrix4::perspective_matrix(float fov, float aspect_ratio, float near, float far)
{
    Matrix4 result(0.0f);

    auto const f = std::tan(0.5f * fov);

    result[0][0] = 1.0f / (aspect_ratio * f);
    result[1][1] = 1.0f / f;
    result[2][2] = far / (near - far);
    result[2][3] = -1.0f;
    result[3][2] = -(far * near) / (far - near);
    return result;
}

constexpr Matrix4 Matrix4::perspective_matrix(float fov, float aspect_ratio, float near)
{
    Matrix4 result(0.0f);

    auto const range = std::tan(0.5f * fov) * near;
    auto const left = -range * aspect_ratio;
    auto const right = range * aspect_ratio;
    auto const bottom = -range;
    auto const top = range;

    result[0][0] = (2.0f * near) / (right - left);
    result[1][1] = (2.0f * near) / (top - bottom);
    result[2][2] = -1.0f;
    result[2][3] = -1.0f;
    result[3][2] = -2.0f * near;
    return result;
}

constexpr Matrix4 Matrix4::orthographic_matrix(float left, float right, float bottom, float top, float near, float far)
{
    Matrix4 result;

    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    result[2][2] = -1.0f / (far - near);
    result[2][3] = near / (far - near);
    return result;
}

constexpr Matrix4 Matrix4::frustum_matrix(float left, float right, float bottom, float top, float near, float far)
{
    Matrix4 result(0.0f);

    result[0][0] = (2.0f * near) / (right - left);
    result[1][1] = (2.0f * near) / (top - bottom);
    result[2][0] = (right + left) / (right - left);
    result[2][1] = (top + bottom) / (top - bottom);
    result[2][3] = -1.0f;
    result[2][2] = far / (near - far);
    result[3][2] = -(far * near) / (far - near);
    return result;
}

constexpr Matrix4 Matrix4::view_matrix(Vector3f const& position, Vector3f const& rotation)
{
    Matrix4 result;

    result = result.rotate(rotation.x, Vector3f::right);
    result = result.rotate(rotation.y, Vector3f::up);
    result = result.rotate(rotation.z, Vector3f::front);
    result = result.translate(-position);
    return result;
}

constexpr Vector3f
Matrix4::project(Vector3f const& world_space, Matrix4 const& view_matrix, Matrix4 const& projection_matrix)
{
    Vector4f point4(world_space, 1.0f);
    point4 = view_matrix.transform(point4);
    point4 = projection_matrix.transform(point4);

    Vector3f result(point4);
    result.x /= result.z;
    result.y /= result.z;
    return result;
}

constexpr Vector3f Matrix4::unproject(
    [[maybe_unused]] Vector3f const& screen_space, [[maybe_unused]] Matrix4 const& view_matrix,
    [[maybe_unused]] Matrix4 const& projection_matrix
)
{
    Vector3f result;
    // TODO: Write unproject method
    return result;
}

constexpr Matrix4 Matrix4::look_at(Vector3f const& eye, Vector3f const& centre, Vector3f const& up)
{
    Matrix4 result;

    auto const f = (centre - eye).normalize();
    auto const s = f.cross(up).normalize();
    auto const u = s.cross(f);

    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;
    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;
    result[0][2] = -f.x;
    result[1][2] = -f.y;
    result[2][2] = -f.z;
    result[3][0] = -s.dot(eye);
    result[3][1] = -u.dot(eye);
    result[3][2] = f.dot(eye);
    return result;
}

constexpr Matrix4::row_t const& Matrix4::operator[](uint32_t index) const
{
    assert(index < COUNT);
    return rows[index];
}

constexpr Matrix4::row_t& Matrix4::operator[](uint32_t index)
{
    assert(index < COUNT);
    return rows[index];
}

constexpr bool Matrix4::operator==(Matrix4 const& other) const
{
    return rows[0] == other.rows[0] && rows[1] == other.rows[1] && rows[2] == other[2] && rows[3] == other[3];
}

constexpr bool Matrix4::operator!=(Matrix4 const& other) const
{
    return !operator==(other);
}

constexpr Matrix4 Matrix4::operator-() const
{
    return negate();
}

constexpr Matrix4 operator+(Matrix4 const& lhs, Matrix4 const& rhs)
{
    return lhs.add(rhs);
}

constexpr Matrix4 operator-(Matrix4 const& lhs, Matrix4 const& rhs)
{
    return lhs.subtract(rhs);
}

constexpr Matrix4 operator*(Matrix4 const& lhs, Matrix4 const& rhs)
{
    return lhs.multiply(rhs);
}

constexpr Matrix4 operator/(Matrix4 const& lhs, Matrix4 const& rhs)
{
    return lhs.divide(rhs);
}

constexpr Matrix4 operator*(Matrix4::row_t const& lhs, Matrix4 const& rhs)
{
    return rhs.scale(lhs);
}

constexpr Matrix4 operator/(Matrix4::row_t const& lhs, Matrix4 const& rhs)
{
    return rhs.scale(1.0f / lhs);
}

constexpr Matrix4 operator*(Matrix4 const& lhs, Matrix4::row_t const& rhs)
{
    return lhs.scale(rhs);
}

constexpr Matrix4 operator/(Matrix4 const& lhs, Matrix4::row_t const& rhs)
{
    return lhs.scale(1.0f / rhs);
}

constexpr Matrix4 operator*(float lhs, Matrix4 const& rhs)
{
    return rhs.scale(Vector2(lhs));
}

constexpr Matrix4 operator/(float lhs, Matrix4 const& rhs)
{
    return rhs.scale(1.0f / Vector2(lhs));
}

constexpr Matrix4 operator*(Matrix4 const& lhs, float rhs)
{
    return lhs.scale(Vector2(rhs));
}

constexpr Matrix4 operator/(Matrix4 const& lhs, float rhs)
{
    return lhs.scale(1.0f / Vector2(rhs));
}

constexpr Matrix4& Matrix4::operator+=(Matrix4 const& other)
{
    return *this = add(other);
}

constexpr Matrix4& Matrix4::operator-=(Matrix4 const& other)
{
    return *this = subtract(other);
}

constexpr Matrix4& Matrix4::operator*=(Matrix4 const& other)
{
    return *this = multiply(other);
}

constexpr Matrix4& Matrix4::operator/=(Matrix4 const& other)
{
    return *this = divide(other);
}

constexpr Matrix4& Matrix4::operator*=(Matrix4::row_t const& other)
{
    return *this = scale(other);
}

constexpr Matrix4& Matrix4::operator/=(Matrix4::row_t const& other)
{
    return *this = scale(1.0f / other);
}

constexpr Matrix4& Matrix4::operator*=(float other)
{
    return *this = scale(Vector2(other));
}

constexpr Matrix4& Matrix4::operator/=(float other)
{
    return *this = scale(1.0f / Vector2(other));
}

constexpr std::ostream& operator<<(std::ostream& stream, Matrix4 const& matrix)
{
    return stream << matrix[0] << ", " << matrix[1] << ", " << matrix[2] << ", " << matrix[3];
}
}