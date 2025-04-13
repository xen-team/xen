#pragma once

#include "quaternion.hpp"
#include "math.hpp"

namespace xen {

constexpr Quaternion::Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

constexpr Quaternion::Quaternion(Vector3f const& source)
{
    auto sx = std::sin(source.x * 0.5f);
    auto cx = Math::cos_from_sin(sx, source.x * 0.5f);
    auto sy = std::sin(source.y * 0.5f);
    auto cy = Math::cos_from_sin(sy, source.y * 0.5f);
    auto sz = std::sin(source.z * 0.5f);
    auto cz = Math::cos_from_sin(sz, source.z * 0.5f);

    auto cycz = cy * cz;
    auto sysz = sy * sz;
    auto sycz = sy * cz;
    auto cysz = cy * sz;

    w = cx * cycz - sx * sysz;
    x = sx * cycz + cx * sysz;
    y = cx * sycz - sx * cysz;
    z = cx * cysz + sx * sycz;
}

constexpr Quaternion::Quaternion(Matrix4 const& source)
{
    auto diagonal = source[0][0] + source[1][1] + source[2][2];

    if (diagonal > 0.0f) {
        auto w4 = std::sqrt(diagonal + 1.0f) * 2.0f;
        w = w4 / 4.0f;
        x = (source[2][1] - source[1][2]) / w4;
        y = (source[0][2] - source[2][0]) / w4;
        z = (source[1][0] - source[0][1]) / w4;
    }
    else if ((source[0][0] > source[1][1]) && (source[0][0] > source[2][2])) {
        auto x4 = std::sqrt(1.0f + source[0][0] - source[1][1] - source[2][2]) * 2.0f;
        w = (source[2][1] - source[1][2]) / x4;
        x = x4 / 4.0f;
        y = (source[0][1] + source[1][0]) / x4;
        z = (source[0][2] + source[2][0]) / x4;
    }
    else if (source[1][1] > source[2][2]) {
        auto y4 = std::sqrt(1.0f + source[1][1] - source[0][0] - source[2][2]) * 2.0f;
        w = (source[0][2] - source[2][0]) / y4;
        x = (source[0][1] + source[1][0]) / y4;
        y = y4 / 4.0f;
        z = (source[1][2] + source[2][1]) / y4;
    }
    else {
        auto z4 = std::sqrt(1.0f + source[2][2] - source[0][0] - source[1][1]) * 2.0f;
        w = (source[1][0] - source[0][1]) / z4;
        x = (source[0][2] + source[2][0]) / z4;
        y = (source[1][2] + source[2][1]) / z4;
        z = z4 / 4.0f;
    }
}

constexpr Quaternion::Quaternion(Vector3f const& axis_x, Vector3f const& axis_y, Vector3f const& axis_z)
{
    Matrix4 rotation;

    rotation[0][0] = axis_x.x;
    rotation[1][0] = axis_x.y;
    rotation[2][0] = axis_x.z;
    rotation[0][1] = axis_y.x;
    rotation[1][1] = axis_y.y;
    rotation[2][1] = axis_y.z;
    rotation[0][2] = axis_z.x;
    rotation[1][2] = axis_z.y;
    rotation[2][2] = axis_z.z;
    *this = rotation;
}

constexpr Quaternion Quaternion::multiply_inverse(Quaternion const& other) const
{
    auto n = other.length_squared();
    n = n == 0.0f ? n : 1.0f / n;
    return {
        (x * other.w - w * other.x - y * other.z + z * other.y) * n,
        (y * other.w - w * other.y - z * other.x + x * other.z) * n,
        (z * other.w - w * other.z - x * other.y + y * other.x) * n,
        (w * other.w + x * other.x + y * other.y + z * other.z) * n
    };
}

constexpr float Quaternion::dot(Quaternion const& other) const
{
    return w * other.w + x * other.x + y * other.y + z * other.z;
}

constexpr Quaternion Quaternion::slerp(Quaternion const& other, float progression) const
{
    auto const cosom = x * other.x + y * other.y + z * other.z + w * other.w;
    auto const absCosom = std::abs(cosom);
    float scale0 = NAN, scale1 = NAN;

    if (1.0f - absCosom > 1E-6f) {
        auto sinSqr = 1.0f - (absCosom * absCosom);
        auto sinom = 1.0f / std::sqrt(sinSqr);
        auto omega = std::atan2(sinSqr * sinom, absCosom);
        scale0 = std::sin((1.0f - progression) * omega) * sinom;
        scale1 = std::sin(progression * omega) * sinom;
    }
    else {
        scale0 = 1.0f - progression;
        scale1 = progression;
    }

    scale1 = cosom >= 0.0f ? scale1 : -scale1;
    Quaternion result;
    result.x = scale0 * x + scale1 * other.x;
    result.y = scale0 * y + scale1 * other.y;
    result.z = scale0 * z + scale1 * other.z;
    result.w = scale0 * w + scale1 * other.w;
    return result;
}

constexpr Quaternion Quaternion::normalize() const
{
    auto const l = length();
    return {x / l, y / l, z / l, w / l};
}

constexpr float Quaternion::length_squared() const
{
    return x * x + y * y + z * z + w * w;
}

constexpr float Quaternion::length() const
{
    return std::sqrt(length_squared());
}

constexpr float Quaternion::max_component() const
{
    return std::max({x, y, z, w});
}

constexpr float Quaternion::min_component() const
{
    return std::min({x, y, z, w});
}

constexpr Matrix4 Quaternion::to_matrix() const
{
    auto const w2 = w * w;
    auto const x2 = x * x;
    auto const y2 = y * y;
    auto const z2 = z * z;
    auto const zw = z * w;
    auto const xy = x * y;
    auto const xz = x * z;
    auto const yw = y * w;
    auto const yz = y * z;
    auto const xw = x * w;

    Matrix4 result;
    result[0][0] = w2 + x2 - z2 - y2;
    result[0][1] = xy + zw + zw + xy;
    result[0][2] = xz - yw + xz - yw;
    result[1][0] = -zw + xy - zw + xy;
    result[1][1] = y2 - z2 + w2 - x2;
    result[1][2] = yz + yz + xw + xw;
    result[2][0] = yw + xz + xz + yw;
    result[2][1] = yz + yz - xw - xw;
    result[2][2] = z2 - y2 - x2 + w2;
    return result;
}

constexpr Matrix4 Quaternion::to_rotation_matrix() const
{
    auto const xy = x * y;
    auto const xz = x * z;
    auto const xw = x * w;
    auto const yz = y * z;
    auto const yw = y * w;
    auto const zw = z * w;
    auto const x_squared = x * x;
    auto const y_squared = y * y;
    auto const z_squared = z * z;

    Matrix4 result;
    result[0][0] = 1.0f - 2.0f * (y_squared + z_squared);
    result[0][1] = 2.0f * (xy - zw);
    result[0][2] = 2.0f * (xz + yw);
    result[0][3] = 0.0f;
    result[1][0] = 2.0f * (xy + zw);
    result[1][1] = 1.0f - 2.0f * (x_squared + z_squared);
    result[1][2] = 2.0f * (yz - xw);
    result[1][3] = 0.0f;
    result[2][0] = 2.0f * (xz - yw);
    result[2][1] = 2.0f * (yz + xw);
    result[2][2] = 1.0f - 2.0f * (x_squared + y_squared);
    return result;
}

constexpr static Quaternion from_rotation_matrix(Matrix4 const& rotation_matrix)
{
    float const m00 = rotation_matrix[0][0];
    float const m11 = rotation_matrix[1][1];
    float const m22 = rotation_matrix[2][2];
    float const w = std::sqrt(std::max(0.0f, 1.0f + m00 + m11 + m22)) / 2.0f;

    float const x = (rotation_matrix[2][1] - rotation_matrix[1][2]) / (4.0f * w);
    float const y = (rotation_matrix[0][2] - rotation_matrix[2][0]) / (4.0f * w);
    float const z = (rotation_matrix[1][0] - rotation_matrix[0][1]) / (4.0f * w);

    return Quaternion(x, y, z, w).normalize();
}

constexpr Vector3f Quaternion::to_euler() const
{
    Vector3f result;
    result.x = std::atan2(2.0f * (x * w - y * z), 1.0f - (2.0f * (x * x + y * y)));
    result.y = std::asin(2.0f * (x * z + y * w));
    result.z = std::atan2(2.0f * (z * w - x * y), 1.0f - (2.0f * (y * y + z * z)));
    return result;
}

constexpr float Quaternion::operator[](uint32_t index) const
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    case 3:
        return w;
    default:
        throw std::runtime_error("Quaternion index out of bounds!");
    }
}

constexpr float& Quaternion::operator[](uint32_t index)
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    case 3:
        return w;
    default:
        throw std::runtime_error("Quaternion index out of bounds!");
    }
}

constexpr bool Quaternion::operator==(Quaternion const& rhs) const
{
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

constexpr bool Quaternion::operator!=(Quaternion const& rhs) const
{
    return !operator==(rhs);
}

constexpr Quaternion Quaternion::operator-() const
{
    return {-x, -y, -z, -w};
}

constexpr Quaternion operator+(Quaternion const& lhs, Quaternion const& rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w};
}

constexpr Quaternion operator-(Quaternion const& lhs, Quaternion const& rhs)
{
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w};
}

constexpr Quaternion operator*(Quaternion const& lhs, Quaternion const& rhs)
{
    return {
        lhs.x * rhs.w + lhs.w * rhs.x + lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.y * rhs.w + lhs.w * rhs.y + lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.z * rhs.w + lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x,
        lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
    };
}

constexpr Vector3f operator*(Vector3f const& lhs, Quaternion const& rhs)
{
    Vector3f const q(rhs.x, rhs.y, rhs.z);
    auto const cross1 = q.cross(lhs);
    auto const cross2 = q.cross(cross1);
    return lhs + 2.0f * (cross1 * rhs.w + cross2);
}

constexpr Vector3f operator*(Quaternion const& lhs, Vector3f const& rhs)
{
    return rhs * lhs;
}

constexpr Quaternion operator*(float lhs, Quaternion const& rhs)
{
    return {rhs.x * lhs, rhs.y * lhs, rhs.z * lhs, rhs.w * lhs};
}

constexpr Quaternion operator*(Quaternion const& lhs, float rhs)
{
    return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs};
}

constexpr Quaternion& Quaternion::operator*=(Quaternion const& rhs)
{
    return *this = *this * rhs;
}

constexpr Quaternion& Quaternion::operator*=(float rhs)
{
    return *this = *this * rhs;
}

constexpr std::ostream& operator<<(std::ostream& stream, Quaternion const& quaternion)
{
    return stream << quaternion.x << ", " << quaternion.y << ", " << quaternion.z << ", " << quaternion.w;
}
}