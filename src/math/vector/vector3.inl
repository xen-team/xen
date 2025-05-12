#pragma once

#include "vector3.hpp"

#include <math/math.hpp>

namespace xen {
template <typename T>
constexpr Vector3<T>::Vector3(T const& a) : x(a), y(a), z(a)
{
}

template <typename T>
constexpr Vector3<T>::Vector3(T const& x, T const& y, T const& z) : x(x), y(y), z(z)
{
}

template <typename T>
template <typename K, typename J, typename H>
constexpr Vector3<T>::Vector3(K const& x, J const& y, H const& z) :
    x(static_cast<T>(x)), y(static_cast<T>(y)), z(static_cast<T>(z))
{
}

template <typename T>
template <typename K>
constexpr Vector3<T>::Vector3(Vector2<K> const& src) : x(static_cast<T>(src.x)), y(static_cast<T>(src.y)), z(0)
{
}

template <typename T>
template <typename K, typename J>
constexpr Vector3<T>::Vector3(Vector2<K> const& src, J const& z) :
    x(static_cast<T>(src.x)), y(static_cast<T>(src.y)), z(static_cast<T>(z))
{
}

template <typename T>
template <typename K>
constexpr Vector3<T>::Vector3(Vector4<K> const& src) :
    x(static_cast<T>(src.x)), y(static_cast<T>(src.y)), z(static_cast<T>(src.z))
{
}

template <typename T>
template <typename U>
constexpr Vector3<T>::Vector3(Vector3<U> const& src) :
    x(static_cast<T>(src.x)), y(static_cast<T>(src.y)), z(static_cast<T>(src.z))
{
}

template <typename T>
constexpr Vector3<T>::Vector3(Vector3<T> const& src) : x(src.x), y(src.y), z(src.z)
{
}

template <typename T>
constexpr Vector3<T>::Vector3(Vector3<T>&& src) noexcept : x(std::move(src.x)), y(std::move(src.y)), z(std::move(src.z))
{
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::add(Vector3<U> const& other) const
{
    return Vector3<decltype(x + other.x)>(x + other.x, y + other.y, z + other.z);
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::subtract(Vector3<U> const& other) const
{
    return Vector3<decltype(x - other.x)>(x - other.x, y - other.y, z - other.z);
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::multiply(Vector3<U> const& other) const
{
    return Vector3<decltype(x * other.x)>(x * other.x, y * other.y, z * other.z);
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::divide(Vector3<U> const& other) const
{
    return Vector3<decltype(x / other.x)>(x / other.x, y / other.y, z / other.z);
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::angle(Vector3<U> const& other) const
{
    auto dls = dot(other) / (length() * other.length());

    if (dls < -1) {
        dls = -1;
    }
    else if (dls > 1) {
        dls = 1;
    }

    return std::acos(dls);
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::dot(Vector3<U> const& other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z);
}

template <typename T>
template <typename K>
constexpr auto Vector3<T>::cross(Vector3<K> const& other) const
{
    return Vector3<decltype(x * other.x)>(
        (y * other.z) - (z * other.y), (other.x * z) - (other.z * x), (x * other.y) - (y * other.x)
    );
}

template <typename T>
template <typename U, typename V>
constexpr auto Vector3<T>::lerp(Vector3<U> const& other, V const& progression) const
{
    auto ta = *this * (1 - progression);
    auto tb = other * progression;
    return ta + tb;
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::scale(U const& scalar) const
{
    return Vector3<decltype(x * scalar)>(x * scalar, y * scalar, z * scalar);
}

// template <typename T>
// template <typename U>
// constexpr auto Vector3<T>::rotate(U const& angle) const
// {
//     auto matrix = Matrix4::tranform_matrix(zero, angle, one);
//     return matrix.Transform(Vector4f(*this));
// }

template <typename T>
constexpr auto Vector3<T>::normalize() const
{
    if constexpr (std::is_floating_point_v<T>) {
        auto const l = length();

        if (l == 0) {
            return *this;
            throw std::runtime_error("Can't normalize a zero length vector");
        }

        return *this / l;
    }
    else {
        return *this;
    }
}

template <typename T>
constexpr auto Vector3<T>::length_squared() const
{
    return (x * x) + (y * y) + (z * z);
}

template <typename T>
constexpr auto Vector3<T>::length() const
{
    return std::sqrt(length_squared());
}

template <typename T>
constexpr auto Vector3<T>::abs() const
{
    if constexpr (std::is_signed_v<T>) {
        return Vector3<T>(std::abs(x), std::abs(y), std::abs(z));
    }
    else {
        return (*this);
    }
}

template <typename T>
constexpr auto Vector3<T>::min() const
{
    return std::min({x, y, z});
}

template <typename T>
constexpr auto Vector3<T>::max() const
{
    return std::max({x, y, z});
}

template <typename T>
constexpr auto Vector3<T>::min_max() const
{
    return std::minmax({x, y, z});
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::min(Vector3<U> const& other) const
{
    using type = decltype(x + other.x);
    return Vector3<type>(std::min<type>(x, other.x), std::min<type>(y, other.y), std::min<type>(z, other.z));
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::max(Vector3<U> const& other) const
{
    using type = decltype(x + other.x);
    return Vector3<type>(std::max<type>(x, other.x), std::max<type>(y, other.y), std::max<type>(z, other.z));
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::distance_squared(Vector3<U> const& other) const
{
    auto const dx = x - other.x;
    auto const dy = y - other.y;
    auto const dz = z - other.z;
    return (dx * dx) + (dy * dy) + (dz * dz);
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::distance(Vector3<U> const& other) const
{
    return std::sqrt(distance_squared(other));
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::distance_vector(Vector3<U> const& other) const
{
    return (*this - other) * (*this - other);
}

template <typename T>
template <typename K, typename J>
constexpr auto Vector3<T>::smooth_damp(Vector3<K> const& target, Vector3<J> const& rate) const
{
    return Math::smooth_damp(*this, target, rate);
}

template <typename T>
constexpr auto Vector3<T>::cartesian_to_polar() const
{
    auto const radius = std::sqrt(length_squared());
    auto const theta = std::atan2(y, x);
    auto const phi = std::atan2(std::sqrt((x * x) + (y * y)), z);
    return Vector3<decltype(radius)>(radius, theta, phi);
}

template <typename T>
constexpr auto Vector3<T>::polar_to_cartesian() const
{
    auto const x1 = x * std::sin(z) * std::cos(y);
    auto const y1 = x * std::sin(z) * std::sin(y);
    auto const z1 = x * std::cos(z);
    return Vector3<decltype(x1)>(x1, y1, z1);
}

template <typename T>
constexpr std::size_t Vector3<T>::hash(std::size_t seed) const
{
    std::hash<T> hasher{};

    seed ^= hasher(x) + 0x9e3779b9 + (seed << 6u) + (seed >> 2u);
    seed ^= hasher(y) + 0x9e3779b9 + (seed << 6u) + (seed >> 2u);
    seed ^= hasher(z) + 0x9e3779b9 + (seed << 6u) + (seed >> 2u);

    return seed;
}

template <typename T>
constexpr Vector3<T>& Vector3<T>::operator=(Vector3<T> const& other)
{
    this->x = other.x;
    this->y = other.y;
    this->z = other.z;
    return *this;
}

template <typename T>
constexpr Vector3<T>& Vector3<T>::operator=(Vector3<T>&& other) noexcept
{
    this->x = std::move(other.x);
    this->y = std::move(other.y);
    this->z = std::move(other.z);
    return *this;
}

template <typename T>
template <typename U>
constexpr Vector3<T>& Vector3<T>::operator=(Vector3<U> const& other)
{
    this->x = static_cast<T>(other.x);
    this->y = static_cast<T>(other.y);
    this->z = static_cast<T>(other.z);
    return *this;
}

template <typename T>
constexpr T const& Vector3<T>::operator[](uint32_t index) const
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        throw std::runtime_error("Vector3 index out of bounds!");
    }
}

template <typename T>
constexpr T& Vector3<T>::operator[](uint32_t index)
{
    switch (index) {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        throw std::runtime_error("Vector3 index out of bounds!");
    }
}

template <typename T>
template <typename U>
constexpr bool Vector3<T>::operator==(Vector3<U> const& other) const
{
    return x == other.x && y == other.y && z == other.z;
}

template <typename T>
template <typename U>
constexpr bool Vector3<T>::operator!=(Vector3<U> const& other) const
{
    return !operator==(other);
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::operator-() const -> std::enable_if_t<std::is_signed_v<U>, Vector3<T>>
{
    return {-x, -y, -z};
}

template <typename T>
template <typename U>
constexpr auto Vector3<T>::operator~() const -> std::enable_if_t<std::is_integral_v<U>, Vector3<T>>
{
    return {~x, ~y, ~z};
}

template <typename T>
template <typename U>
constexpr Vector3<T>& Vector3<T>::operator+=(Vector3<U> const& other)
{
    return *this = add(other);
}

template <typename T>
template <typename U>
constexpr Vector3<T>& Vector3<T>::operator-=(Vector3<U> const& other)
{
    return *this = subtract(other);
}

template <typename T>
template <typename U>
constexpr Vector3<T>& Vector3<T>::operator*=(Vector3<U> const& other)
{
    return *this = multiply(other);
}

template <typename T>
template <typename U>
constexpr Vector3<T>& Vector3<T>::operator/=(Vector3<U> const& other)
{
    return *this = divide(other);
}

template <typename T>
constexpr Vector3<T>& Vector3<T>::operator+=(T const& other)
{
    return *this = add(Vector3<T>(other));
}

template <typename T>
constexpr Vector3<T>& Vector3<T>::operator-=(T const& other)
{
    return *this = subtract(Vector3<T>(other));
}

template <typename T>
constexpr Vector3<T>& Vector3<T>::operator*=(T const& other)
{
    return *this = multiply(Vector3<T>(other));
}

template <typename T>
constexpr Vector3<T>& Vector3<T>::operator/=(T const& other)
{
    return *this = divide(Vector3<T>(other));
}

template <typename U>
static std::ostream& operator<<(std::ostream& stream, Vector3<U> const& vector)
{
    return stream << vector.x << ", " << vector.y << ", " << vector.z;
}

template <typename U, typename V>
constexpr static auto operator+(Vector3<U> const& lhs, Vector3<V> const& rhs)
{
    return lhs.add(rhs);
}

template <typename U, typename V>
constexpr static auto operator-(Vector3<U> const& lhs, Vector3<V> const& rhs)
{
    return lhs.subtract(rhs);
}

template <typename U, typename V>
constexpr static auto operator*(Vector3<U> const& lhs, Vector3<V> const& rhs)
{
    return lhs.multiply(rhs);
}

template <typename U, typename V>
constexpr static auto operator/(Vector3<U> const& lhs, Vector3<V> const& rhs)
{
    return lhs.divide(rhs);
}

template <typename U, typename V>
constexpr static auto operator+(U const& lhs, Vector3<V> const& rhs)
{
    return Vector3<U>(lhs).add(rhs);
}

template <typename U, typename V>
constexpr static auto operator-(U const& lhs, Vector3<V> const& rhs)
{
    return Vector3<U>(lhs).subtract(rhs);
}

template <typename U, typename V>
constexpr static auto operator*(U const& lhs, Vector3<V> const& rhs)
{
    return Vector3<U>(lhs).multiply(rhs);
}

template <typename U, typename V>
constexpr static auto operator/(U const& lhs, Vector3<V> const& rhs)
{
    return Vector3<U>(lhs).divide(rhs);
}

template <typename U, typename V>
constexpr static auto operator+(Vector3<U> const& lhs, V const& rhs)
{
    return lhs.add(Vector3<V>(rhs));
}

template <typename U, typename V>
constexpr static auto operator-(Vector3<U> const& lhs, V const& rhs)
{
    return lhs.subtract(Vector3<V>(rhs));
}

template <typename U, typename V>
constexpr static auto operator*(Vector3<U> const& lhs, V const& rhs)
{
    return lhs.multiply(Vector3<V>(rhs));
}

template <typename U, typename V>
constexpr static auto operator/(Vector3<U> const& lhs, V const& rhs)
{
    return lhs.divide(Vector3<V>(rhs));
}

template <Integral U, Integral V>
constexpr static Vector3<V> operator&(Vector3<U> const& lhs, Vector3<V> const& rhs)
{
    return {lhs.x & rhs.x, lhs.y & rhs.y, lhs.z & rhs.z};
}

template <Integral U, Integral V>
constexpr static Vector3<V> operator|(Vector3<U> const& lhs, Vector3<V> const& rhs)
{
    return {lhs.x | rhs.x, lhs.y | rhs.y, lhs.z | rhs.z};
}

template <Integral U, Integral V>
constexpr static Vector3<V> operator>>(Vector3<U> const& lhs, Vector3<V> const& rhs)
{
    return {lhs.x >> rhs.x, lhs.y >> rhs.y, lhs.z >> rhs.z};
}

template <Integral U, Integral V>
constexpr static Vector3<V> operator<<(Vector3<U> const& lhs, Vector3<V> const& rhs)
{
    return {lhs.x << rhs.x, lhs.y << rhs.y, lhs.z << rhs.z};
}

template <Integral U, Integral V>
constexpr static Vector3<V> operator&(Vector3<U> const& lhs, V const& rhs)
{
    return {lhs.x & rhs, lhs.y & rhs, lhs.z & rhs};
}

template <Integral U, Integral V>
constexpr static Vector3<V> operator|(Vector3<U> const& lhs, V const& rhs)
{
    return {lhs.x | rhs, lhs.y | rhs, lhs.z | rhs};
}

template <Integral U, Integral V>
constexpr static Vector3<V> operator>>(Vector3<U> const& lhs, V const& rhs)
{
    return {lhs.x >> rhs, lhs.y >> rhs, lhs.z >> rhs};
}

template <Integral U, Integral V>
constexpr static Vector3<V> operator<<(Vector3<U> const& lhs, V const& rhs)
{
    return {lhs.x << rhs, lhs.y << rhs, lhs.z << rhs};
}
}