#pragma once

#include "vector2.hpp"

#include "utils/concepts.hpp"

#include <cmath>

namespace xen {
template <typename T>
constexpr Vector2<T>::Vector2(T const& a) : x(a), y(a)
{
}

template <typename T>
constexpr Vector2<T>::Vector2(T const& x, T const& y) : x(x), y(y)
{
}

template <typename T>
template <typename K, typename J>
constexpr Vector2<T>::Vector2(K const& x, J const& y) : x(static_cast<T>(x)), y(static_cast<T>(y))
{
}

template <typename T>
template <typename U>
constexpr Vector2<T>::Vector2(Vector3<U> const& src) : x(static_cast<T>(src.x)), y(static_cast<T>(src.y))
{
}

template <typename T>
template <typename U>
constexpr Vector2<T>::Vector2(Vector4<U> const& src) : x(static_cast<T>(src.x)), y(static_cast<T>(src.y))
{
}

template <typename T>
template <typename U>
constexpr Vector2<T>::Vector2(Vector2<U> const& src) : x(static_cast<T>(src.x)), y(static_cast<T>(src.y))
{
}

template <typename T>
constexpr Vector2<T>::Vector2(Vector2<T> const& src) : x(src.x), y(src.y)
{
}

template <typename T>
constexpr Vector2<T>::Vector2(Vector2<T>&& src) noexcept : x(std::move(src.x)), y(std::move(src.y))
{
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::add(Vector2<U> const& other) const
{
    return Vector2<decltype(x + other.x)>(x + other.x, y + other.y);
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::subtract(Vector2<U> const& other) const
{
    return Vector2<decltype(x - other.x)>(x - other.x, y - other.y);
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::multiply(Vector2<U> const& other) const
{
    return Vector2<decltype(x * other.x)>(x * other.x, y * other.y);
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::divide(Vector2<U> const& other) const
{
    return Vector2<decltype(x / other.x)>(x / other.x, y / other.y);
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::angle(Vector2<U> const& other) const
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
constexpr auto Vector2<T>::dot(Vector2<U> const& other) const
{
    return (x * other.x) + (y * other.y);
}

template <typename T>
template <typename U, typename V>
constexpr auto Vector2<T>::lerp(Vector2<U> const& other, V const& progression) const
{
    auto ta = *this * (1 - progression);
    auto tb = other * progression;
    return ta + tb;
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::scale(U const& scalar) const
{
    return Vector2<decltype(x * scalar)>(x * scalar, y * scalar);
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::rotate(U const& angle) const
{
    return Vector2<decltype(x * angle)>(
        (x * std::cos(angle)) - (y * std::sin(angle)), (x * std::sin(angle)) + (y * std::cos(angle))
    );
}

template <typename T>
template <typename U, typename V>
constexpr auto Vector2<T>::rotate(U const& angle, Vector2<V> const& rotation_axis) const
{
    auto x1 = ((x - rotation_axis.x) * std::cos(angle)) - ((y - rotation_axis.y) * std::sin(angle) + rotation_axis.x);
    auto y1 = ((x - rotation_axis.x) * std::sin(angle)) + ((y - rotation_axis.y) * std::cos(angle) + rotation_axis.y);
    return Vector2<decltype(x1)>(x1, y1);
}

template <typename T>
constexpr auto Vector2<T>::normalize() const
{
    auto const l = length();

    if (l == 0) {
        throw std::runtime_error("Can't normalize a zero length vector");
    }

    return *this / l;
}

template <typename T>
constexpr auto Vector2<T>::length_squared() const
{
    return (x * x) + (y * y);
}

template <typename T>
constexpr auto Vector2<T>::length() const
{
    return std::sqrt(length_squared());
}

template <typename T>
constexpr auto Vector2<T>::abs() const
{
    return Vector2<T>(std::abs(x), std::abs(y));
}

template <typename T>
constexpr auto Vector2<T>::min() const
{
    return std::min({x, y});
}

template <typename T>
constexpr auto Vector2<T>::max() const
{
    return std::max({x, y});
}

template <typename T>
constexpr auto Vector2<T>::min_max() const
{
    return std::minmax({x, y});
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::min(Vector2<U> const& other) const
{
    using type = decltype(x + other.x);
    return Vector2<type>(std::min<type>(x, other.x), std::min<type>(y, other.y));
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::max(Vector2<U> const& other) const
{
    using type = decltype(x + other.x);
    return Vector2<type>(std::max<type>(x, other.x), std::max<type>(y, other.y));
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::distance_squared(Vector2<U> const& other) const
{
    auto const dx = x - other.x;
    auto const dy = y - other.y;
    return (dx * dx) + (dy * dy);
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::distance(Vector2<U> const& other) const
{
    return std::sqrt(distance_squared(other));
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::distance_vector(Vector2<U> const& other) const
{
    return (*this - other) * (*this - other);
}

template <typename T>
template <typename U>
constexpr bool Vector2<T>::in_triangle(Vector2<U> const& v1, Vector2<U> const& v2, Vector2<U> const& v3) const
{
    auto const b1 = ((x - v2.x) * (v1.y - v2.y) - (v1.x - v2.x) * (y - v2.y)) < 0;
    auto const b2 = ((x - v3.x) * (v2.y - v3.y) - (v2.x - v3.x) * (y - v3.y)) < 0;
    auto const b3 = ((x - v1.x) * (v3.y - v1.y) - (v3.x - v1.x) * (y - v1.y)) < 0;
    return ((b1 == b2) & (b2 == b3));
}

template <typename T>
constexpr auto Vector2<T>::cartesian_to_polar() const
{
    auto const radius = std::sqrt((x * x) + (y * y));
    auto const theta = std::atan2(y, x);
    return Vector2<decltype(radius)>(radius, theta);
}

template <typename T>
constexpr auto Vector2<T>::polar_to_cartesian() const
{
    auto const x1 = x * std::cos(y);
    auto const y1 = x * std::sin(y);
    return Vector2<decltype(x1)>(x1, y1);
}

template <typename T>
constexpr Vector2<T>& Vector2<T>::operator=(Vector2<T> const& other)
{
    this->x = other.x;
    this->y = other.y;
    return *this;
}

template <typename T>
constexpr Vector2<T>& Vector2<T>::operator=(Vector2<T>&& other) noexcept
{
    this->x = std::move(other.x);
    this->y = std::move(other.y);
    return *this;
}

template <typename T>
template <typename U>
constexpr Vector2<T>& Vector2<T>::operator=(Vector2<U> const& other)
{
    this->x = static_cast<T>(other.x);
    this->y = static_cast<T>(other.y);
    return *this;
}

template <typename T>
constexpr T const& Vector2<T>::operator[](uint32_t index) const
{
    if (index == 0) {
        return x;
    }
    if (index == 1) {
        return y;
    }
    throw std::runtime_error("Vector2 index out of bounds!");
}

template <typename T>
constexpr T& Vector2<T>::operator[](uint32_t index)
{
    if (index == 0) {
        return x;
    }
    if (index == 1) {
        return y;
    }
    throw std::runtime_error("Vector2 index out of bounds!");
}

template <typename T>
template <typename U>
constexpr bool Vector2<T>::operator==(Vector2<U> const& other) const
{
    return x == other.x && y == other.y;
}

template <typename T>
template <typename U>
constexpr bool Vector2<T>::operator!=(Vector2<U> const& other) const
{
    return !operator==(other);
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::operator-() const -> std::enable_if_t<std::is_signed_v<U>, Vector2<T>>
{
    return {-x, -y};
}

template <typename T>
template <typename U>
constexpr auto Vector2<T>::operator~() const -> std::enable_if_t<std::is_integral_v<U>, Vector2<T>>
{
    return {~x, ~y};
}

template <typename T>
template <typename U>
constexpr Vector2<T>& Vector2<T>::operator+=(Vector2<U> const& other)
{
    return *this = add(other);
}

template <typename T>
template <typename U>
constexpr Vector2<T>& Vector2<T>::operator-=(Vector2<U> const& other)
{
    return *this = subtract(other);
}

template <typename T>
template <typename U>
constexpr Vector2<T>& Vector2<T>::operator*=(Vector2<U> const& other)
{
    return *this = multiply(other);
}

template <typename T>
template <typename U>
constexpr Vector2<T>& Vector2<T>::operator/=(Vector2<U> const& other)
{
    return *this = divide(other);
}

template <typename T>
constexpr Vector2<T>& Vector2<T>::operator+=(T const& other)
{
    return *this = add(Vector2<T>(other));
}

template <typename T>
constexpr Vector2<T>& Vector2<T>::operator-=(T const& other)
{
    return *this = subtract(Vector2<T>(other));
}

template <typename T>
constexpr Vector2<T>& Vector2<T>::operator*=(T const& other)
{
    return *this = multiply(Vector2<T>(other));
}

template <typename T>
constexpr Vector2<T>& Vector2<T>::operator/=(T const& other)
{
    return *this = divide(Vector2<T>(other));
}

template <typename U>
static std::ostream& operator<<(std::ostream& stream, Vector2<U> const& vector)
{
    return stream << vector.x << ", " << vector.y;
}

template <typename U, typename V>
constexpr static auto operator+(Vector2<U> const& lhs, Vector2<V> const& rhs)
{
    return lhs.add(rhs);
}

template <typename U, typename V>
constexpr static auto operator-(Vector2<U> const& lhs, Vector2<V> const& rhs)
{
    return lhs.subtract(rhs);
}

template <typename U, typename V>
constexpr static auto operator*(Vector2<U> const& lhs, Vector2<V> const& rhs)
{
    return lhs.multiply(rhs);
}

template <typename U, typename V>
constexpr static auto operator/(Vector2<U> const& lhs, Vector2<V> const& rhs)
{
    return lhs.divide(rhs);
}

template <typename U, typename V>
constexpr static auto operator+(U const& lhs, Vector2<V> const& rhs)
{
    return Vector2<U>(lhs).add(rhs);
}

template <typename U, typename V>
constexpr static auto operator-(U const& lhs, Vector2<V> const& rhs)
{
    return Vector2<U>(lhs).subtract(rhs);
}

template <typename U, typename V>
constexpr static auto operator*(U const& lhs, Vector2<V> const& rhs)
{
    return Vector2<U>(lhs).multiply(rhs);
}

template <typename U, typename V>
constexpr static auto operator/(U const& lhs, Vector2<V> const& rhs)
{
    return Vector2<U>(lhs).divide(rhs);
}

template <typename U, typename V>
constexpr static auto operator+(Vector2<U> const& lhs, V const& rhs)
{
    return lhs.add(Vector2<V>(rhs));
}

template <typename U, typename V>
constexpr static auto operator-(Vector2<U> const& lhs, V const& rhs)
{
    return lhs.subtract(Vector2<V>(rhs));
}

template <typename U, typename V>
constexpr static auto operator*(Vector2<U> const& lhs, V const& rhs)
{
    return lhs.multiply(Vector2<V>(rhs));
}

template <typename U, typename V>
constexpr static auto operator/(Vector2<U> const& lhs, V const& rhs)
{
    return lhs.divide(Vector2<V>(rhs));
}

template <Integral U, Integral V>
constexpr static Vector2<V> operator&(Vector2<U> const& lhs, Vector2<V> const& rhs)
{
    return {lhs.x & rhs.x, lhs.y & rhs.y};
}

template <Integral U, Integral V>
constexpr static Vector2<V> operator|(Vector2<U> const& lhs, Vector2<V> const& rhs)
{
    return {lhs.x | rhs.x, lhs.y | rhs.y};
}

template <Integral U, Integral V>
constexpr static Vector2<V> operator>>(Vector2<U> const& lhs, Vector2<V> const& rhs)
{
    return {lhs.x >> rhs.x, lhs.y >> rhs.y};
}

template <Integral U, Integral V>
constexpr static Vector2<V> operator<<(Vector2<U> const& lhs, Vector2<V> const& rhs)
{
    return {lhs.x << rhs.x, lhs.y << rhs.y};
}

template <Integral U, Integral V>
constexpr static Vector2<V> operator&(Vector2<U> const& lhs, V const& rhs)
{
    return {lhs.x & rhs, lhs.y & rhs};
}

template <Integral U, Integral V>
constexpr static Vector2<V> operator|(Vector2<U> const& lhs, V const& rhs)
{
    return {lhs.x | rhs, lhs.y | rhs};
}

template <Integral U, Integral V>
constexpr static Vector2<V> operator>>(Vector2<U> const& lhs, V const& rhs)
{
    return {lhs.x >> rhs, lhs.y >> rhs};
}

template <Integral U, Integral V>
constexpr static Vector2<V> operator<<(Vector2<U> const& lhs, V const& rhs)
{
    return {lhs.x << rhs, lhs.y << rhs};
}
}