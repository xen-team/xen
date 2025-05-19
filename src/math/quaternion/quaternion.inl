#pragma once

#include "quaternion.hpp"

#include <math/math.hpp>
#include <math/angle.hpp>

namespace xen {

constexpr Quaternion::Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

constexpr Quaternion::Quaternion(Vector3f const& euler)
{
    auto const sx = std::sin(euler.x * 0.5f);
    auto const cx = Math::cos_from_sin(sx, euler.x * 0.5f);
    auto const sy = std::sin(euler.y * 0.5f);
    auto const cy = Math::cos_from_sin(sy, euler.y * 0.5f);
    auto const sz = std::sin(euler.z * 0.5f);
    auto const cz = Math::cos_from_sin(sz, euler.z * 0.5f);

    auto const cycz = cy * cz;
    auto const sysz = sy * sz;
    auto const sycz = sy * cz;
    auto const cysz = cy * sz;

    w = cx * cycz - sx * sysz;
    x = sx * cycz + cx * sysz;
    y = cx * sycz - sx * cysz;
    z = cx * cysz + sx * sycz;
}

constexpr Quaternion::Quaternion(Matrix4 const& rotation)
{
    auto const diagonal = rotation[0][0] + rotation[1][1] + rotation[2][2];

    if (diagonal > 0.0f) {
        auto const w4 = std::sqrt(diagonal + 1.0f) * 2.0f;
        w = w4 / 4.0f;
        x = (rotation[2][1] - rotation[1][2]) / w4;
        y = (rotation[0][2] - rotation[2][0]) / w4;
        z = (rotation[1][0] - rotation[0][1]) / w4;
    }
    else if ((rotation[0][0] > rotation[1][1]) && (rotation[0][0] > rotation[2][2])) {
        auto const x4 = std::sqrt(1.0f + rotation[0][0] - rotation[1][1] - rotation[2][2]) * 2.0f;
        w = (rotation[2][1] - rotation[1][2]) / x4;
        x = x4 / 4.0f;
        y = (rotation[0][1] + rotation[1][0]) / x4;
        z = (rotation[0][2] + rotation[2][0]) / x4;
    }
    else if (rotation[1][1] > rotation[2][2]) {
        auto const y4 = std::sqrt(1.0f + rotation[1][1] - rotation[0][0] - rotation[2][2]) * 2.0f;
        w = (rotation[0][2] - rotation[2][0]) / y4;
        x = (rotation[0][1] + rotation[1][0]) / y4;
        y = y4 / 4.0f;
        z = (rotation[1][2] + rotation[2][1]) / y4;
    }
    else {
        auto const z4 = std::sqrt(1.0f + rotation[2][2] - rotation[0][0] - rotation[1][1]) * 2.0f;
        w = (rotation[1][0] - rotation[0][1]) / z4;
        x = (rotation[0][2] + rotation[2][0]) / z4;
        y = (rotation[1][2] + rotation[2][1]) / z4;
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

constexpr Quaternion::Quaternion(Vector3f const& axis, Radians<float> angle)
{
    float const half_angle = angle.value / 2;
    float const sin_angle = std::sin(half_angle);

    w = std::cos(half_angle);

    auto const corrected_axis = axis * sin_angle;
    x = corrected_axis.x;
    y = corrected_axis.y;
    z = corrected_axis.z;
}

constexpr bool Quaternion::is_identity() const
{
    return w == 1.0f && length() == 1.0f;
}

constexpr bool Quaternion::is_pure() const
{
    return w == 0.0f && (x != 0.0f || y != 0.0f || z != 0.0f);
}

constexpr bool Quaternion::is_normalized() const
{
    return abs(length() - 1.0f) < 0.0001f;
}

constexpr Vector3f Quaternion::rotation_axis() const
{
    float const s = sqrt(std::max(1.f - (w * w), 0.f));

    if (s >= 0.0001f) {
        return Vector3f(x / s, y / s, z / s);
    }

    return Vector3f(1.f, 0.f, 0.f);
}

constexpr Quaternion Quaternion::inverse() const
{
    return conjugate() / length_squared();
}

constexpr Quaternion Quaternion::conjugate() const
{
    return {-x, -y, -z, w};
}

constexpr Vector3f Quaternion::rotate_point(Vector3f const& point) const
{
    Vector3f const q(x, y, z);
    Vector3f const t = q.cross(point) * 2.0f;

    return point + (t * w) + q.cross(t);
}

constexpr Vector3f Quaternion::rotate_point(Vector3f const& point, Vector3f const& pivot) const
{
    Vector3f const to_rotate = point - pivot;
    return rotate_point(to_rotate);
}

constexpr Quaternion Quaternion::look_at(Vector3f const& forward, Vector3f const& up)
{
    auto const vector = forward.normalize();
    auto const vector2 = up.cross(vector).normalize();
    auto const vector3 = vector.cross(vector2);
    auto const m00 = vector2.x;
    auto const m01 = vector2.y;
    auto const m02 = vector2.z;
    auto const m10 = vector3.x;
    auto const m11 = vector3.y;
    auto const m12 = vector3.z;
    auto const m20 = vector.x;
    auto const m21 = vector.y;
    auto const m22 = vector.z;

    float const num8 = (m00 + m11) + m22;
    auto quaternion = Quaternion::Identity;
    if (num8 > 0.f) {
        auto num = sqrt(num8 + 1.f);
        quaternion.w = num * 0.5f;
        num = 0.5f / num;
        quaternion.x = (m12 - m21) * num;
        quaternion.y = (m20 - m02) * num;
        quaternion.z = (m01 - m10) * num;
        return quaternion;
    }
    if ((m00 >= m11) && (m00 >= m22)) {
        auto const num7 = sqrt(((1.f + m00) - m11) - m22);
        auto const num4 = 0.5f / num7;
        quaternion.x = 0.5f * num7;
        quaternion.y = (m01 + m10) * num4;
        quaternion.z = (m02 + m20) * num4;
        quaternion.w = (m12 - m21) * num4;
        return quaternion;
    }
    if (m11 > m22) {
        auto const num6 = sqrt(((1.f + m11) - m00) - m22);
        auto const num3 = 0.5f / num6;
        quaternion.x = (m10 + m01) * num3;
        quaternion.y = 0.5f * num6;
        quaternion.z = (m21 + m12) * num3;
        quaternion.w = (m20 - m02) * num3;
        return quaternion;
    }
    auto const num5 = sqrt(((1.f + m22) - m00) - m11);
    auto const num2 = 0.5f / num5;
    quaternion.x = (m20 + m02) * num2;
    quaternion.y = (m21 + m12) * num2;
    quaternion.z = 0.5f * num5;
    quaternion.w = (m01 - m10) * num2;
    return quaternion;
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

constexpr Quaternion Quaternion::lerp(Quaternion const& other, float progression) const
{
    assert("Error: The interpolation coefficient must be between 0 & 1." && (coeff >= 0 && coeff <= 1));

    float const curr_coeff = 1 - progression;
    return {
        x * curr_coeff + other.x * progression, y * curr_coeff + other.y * progression,
        z * curr_coeff + other.z * progression, w * curr_coeff + other.w * progression
    };
}

constexpr Quaternion Quaternion::nlerp(Quaternion const& other, float progression) const
{
    assert("Error: The interpolation coefficient must be between 0 & 1." && (coeff >= 0 && coeff <= 1));

    float const curr_coeff = 1 - progression;
    // A rotation may be represented by two opposite quaternions; should the dot product between those be negative,
    //  one quaternion must be negated. This is made by negating the coefficient
    float const other_coeff = (dot(other) > 0 ? progression : -progression);

    return Quaternion(
               x * curr_coeff + other.x * other_coeff, y * curr_coeff + other.y * other_coeff,
               z * curr_coeff + other.z * other_coeff, w * curr_coeff + other.w * other_coeff
    )
        .normalize();
}

constexpr Quaternion Quaternion::slerp(Quaternion const& other, float progression) const
{
    auto const cosom = x * other.x + y * other.y + z * other.z + w * other.w;
    auto const absCosom = std::abs(cosom);
    float scale0 = NAN, scale1 = NAN;

    if (1.0f - absCosom > 1E-6f) {
        auto const sinSqr = 1.0f - (absCosom * absCosom);
        auto const sinom = 1.0f / std::sqrt(sinSqr);
        auto const omega = std::atan2(sinSqr * sinom, absCosom);
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
    return result.transpose();
}

constexpr Quaternion Quaternion::from_rotation_matrix(Matrix4 const& rotation_matrix)
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
    Vector3f q(lhs.x, lhs.y, lhs.z);
    auto cross1 = q.cross(rhs);
    auto cross2 = q.cross(cross1);
    return rhs + 2.0f * (cross1 * lhs.w + cross2);

    // Adapted from https://fgiesen.wordpress.com/2019/02/09/rotating-a-single-vector-using-a-quaternion/

    Vector3f const quat_vec(lhs.x, lhs.y, lhs.z);
    Vector3f const double_quat_vec_cross = static_cast<float>(2) * quat_vec.cross(rhs);
    return rhs + lhs.w * double_quat_vec_cross + quat_vec.cross(double_quat_vec_cross);

    // return rhs * lhs;
}

constexpr Quaternion operator*(float lhs, Quaternion const& rhs)
{
    return {rhs.x * lhs, rhs.y * lhs, rhs.z * lhs, rhs.w * lhs};
}

constexpr Quaternion operator*(Quaternion const& lhs, float rhs)
{
    return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs};
}

constexpr Quaternion operator/(Quaternion const& lhs, float rhs)
{
    return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs};
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