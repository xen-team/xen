#pragma once

#include "vector4.hpp"

namespace xen {
template <typename T>
constexpr Vector4<T>::Vector4(T const& a) : x(a), y(a), z(a), w(a)
{
}

template <typename T>
constexpr Vector4<T>::Vector4(T const& x, T const& y, T const& z, T const& w) : x(x), y(y), z(z), w(w)
{
}

template <typename T>
template <typename K, typename J, typename H, typename F>
constexpr Vector4<T>::Vector4(K const& x, J const& y, H const& z, F const& w) :
    x(static_cast<T>(x)), y(static_cast<T>(y)), z(static_cast<T>(z)), w(static_cast<T>(w))
{
}

template <typename T>
template <typename K>
constexpr Vector4<T>::Vector4(Vector2<K> const& src) : x{src.x}, y{src.y}
{
}

template <typename T>
template <typename K, typename J>
constexpr Vector4<T>::Vector4(Vector2<K> const& left, Vector2<J> const& right) :
    x(static_cast<T>(left.x)), y(static_cast<T>(left.y)), z(static_cast<T>(right.x)), w(static_cast<T>(right.y))
{
}

template <typename T>
template <typename K, typename J>
constexpr Vector4<T>::Vector4(Vector3<K> const& source, J const& w) :
    x(static_cast<T>(source.x)), y(static_cast<T>(source.y)), z(static_cast<T>(source.z)), w(static_cast<T>(w))
{
}

template <typename T>
template <typename U>
constexpr Vector4<T>::Vector4(Vector4<U> const& src) :
    x(static_cast<T>(src.x)), y(static_cast<T>(src.y)), z(static_cast<T>(src.z)), w(static_cast<T>(src.w))
{
}

template <typename T>
constexpr Vector4<T>::Vector4(Vector4<T> const& src) : x(src.x), y(src.y), z(src.y), w(src.y)
{
}

template <typename T>
constexpr Vector4<T>::Vector4(Vector4<T>&& src) noexcept :
    x(std::move(src.x)), y(std::move(src.y)), z(std::move(src.z)), w(std::move(src.w))
{
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::add(Vector4<U> const& other) const
{
    return Vector4<decltype(x + other.x)>(x + other.x, y + other.y, z + other.z, w + other.w);
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::subtract(Vector4<U> const& other) const
{
    return Vector4<decltype(x - other.x)>(x - other.x, y - other.y, z - other.z, w - other.w);
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::multiply(Vector4<U> const& other) const
{
    return Vector4<decltype(x * other.x)>(x * other.x, y * other.y, z * other.z, w * other.w);
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::divide(Vector4<U> const& other) const
{
    return Vector4<decltype(x / other.x)>(x / other.x, y / other.y, z / other.z, w / other.w);
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::angle(Vector4<U> const& other) const
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
constexpr auto Vector4<T>::dot(Vector4<U> const& other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
}

template <typename T>
template <typename U, typename V>
constexpr auto Vector4<T>::lerp(Vector4<U> const& other, V const& progression) const
{
    auto const ta = *this * (1 - progression);
    auto const tb = other * progression;
    return ta + tb;
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::scale(U const& scalar) const
{
    return Vector4<decltype(x * scalar)>(x * scalar, y * scalar, z * scalar, w * scalar);
}

template <typename T>
constexpr auto Vector4<T>::normalize() const
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
constexpr auto Vector4<T>::length_squared() const
{
    return (x * x) + (y * y) + (z * z) + (w * w);
}

template <typename T>
constexpr auto Vector4<T>::length() const
{
    return std::sqrt(length_squared());
}

template <typename T>
constexpr auto Vector4<T>::abs() const
{
    if constexpr (std::is_signed_v<T>) {
        return Vector4<T>(std::abs(x), std::abs(y), std::abs(z), std::abs(w));
    }
    else {
        return (*this);
    }
}

template <typename T>
constexpr auto Vector4<T>::min() const
{
    return std::min({x, y, z, w});
}

template <typename T>
constexpr auto Vector4<T>::max() const
{
    return std::max({x, y, z, w});
}

template <typename T>
constexpr auto Vector4<T>::min_max() const
{
    return std::minmax({x, y, z, w});
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::min(Vector4<U> const& other) const
{
    using type = decltype(x + other.x);
    return Vector4<type>(
        std::min<type>(x, other.x), std::min<type>(y, other.y), std::min<type>(z, other.z), std::min<type>(w, other.w)
    );
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::max(Vector4<U> const& other) const
{
    using type = decltype(x + other.x);
    return Vector4<type>(
        std::max<type>(x, other.x), std::max<type>(y, other.y), std::max<type>(z, other.z), std::max<type>(w, other.w)
    );
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::distance_squared(Vector4<U> const& other) const
{
    auto const dx = x - other.x;
    auto const dy = y - other.y;
    auto const dz = z - other.z;
    auto const dw = w - other.w;
    return (dx * dx) + (dy * dy) + (dz * dz) + (dw * dw);
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::distance(Vector4<U> const& other) const
{
    return std::sqrt(distance_squared(other));
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::distance_vector(Vector4<U> const& other) const
{
    return (*this - other) * (*this - other);
}

template <typename T>
constexpr std::size_t Vector4<T>::hash(std::size_t seed) const
{
    std::hash<T> hasher{};

    seed ^= hasher(x) + 0x9e3779b9 + (seed << 6u) + (seed >> 2u);
    seed ^= hasher(y) + 0x9e3779b9 + (seed << 6u) + (seed >> 2u);
    seed ^= hasher(z) + 0x9e3779b9 + (seed << 6u) + (seed >> 2u);
    seed ^= hasher(w) + 0x9e3779b9 + (seed << 6u) + (seed >> 2u);

    return seed;
}

template <typename T>
constexpr Vector4<T>& Vector4<T>::operator=(Vector4<T> const& other) = default;

template <typename T>
constexpr Vector4<T>& Vector4<T>::operator=(Vector4<T>&& other) noexcept
{
    this->x = std::move(other.x);
    this->y = std::move(other.y);
    this->z = std::move(other.z);
    this->w = std::move(other.w);
    return *this;
}

template <typename T>
template <typename U>
constexpr Vector4<T>& Vector4<T>::operator=(Vector4<U> const& other)
{
    this->x = static_cast<T>(other.x);
    this->y = static_cast<T>(other.y);
    this->z = static_cast<T>(other.z);
    this->w = static_cast<T>(other.w);
    return *this;
}

template <typename T>
constexpr T const& Vector4<T>::operator[](uint32_t index) const
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
        throw std::runtime_error("Vector4 index out of bounds!");
    }
}

template <typename T>
constexpr T& Vector4<T>::operator[](uint32_t index)
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
        throw std::runtime_error("Vector4 index out of bounds!");
    }
}

template <typename T>
template <typename U>
constexpr bool Vector4<T>::operator==(Vector4<U> const& other) const
{
    return x == other.x && y == other.y && z == other.z && w == other.w;
}

template <typename T>
template <typename U>
constexpr bool Vector4<T>::operator!=(Vector4<U> const& other) const
{
    return !operator==(other);
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::operator-() const -> std::enable_if_t<std::is_signed_v<U>, Vector4<T>>
{
    return {-x, -y, -z, -w};
}

template <typename T>
template <typename U>
constexpr auto Vector4<T>::operator~() const -> std::enable_if_t<std::is_integral_v<U>, Vector4<T>>
{
    return {~x, ~y, ~z, ~w};
}

template <typename T>
template <typename U>
constexpr Vector4<T>& Vector4<T>::operator+=(Vector4<U> const& other)
{
    return *this = add(other);
}

template <typename T>
template <typename U>
constexpr Vector4<T>& Vector4<T>::operator-=(Vector4<U> const& other)
{
    return *this = subtract(other);
}

template <typename T>
template <typename U>
constexpr Vector4<T>& Vector4<T>::operator*=(Vector4<U> const& other)
{
    return *this = multiply(other);
}

template <typename T>
template <typename U>
constexpr Vector4<T>& Vector4<T>::operator/=(Vector4<U> const& other)
{
    return *this = divide(other);
}

template <typename T>
constexpr Vector4<T>& Vector4<T>::operator+=(T const& other)
{
    return *this = add(Vector4<T>(other));
}

template <typename T>
constexpr Vector4<T>& Vector4<T>::operator-=(T const& other)
{
    return *this = subtract(Vector4<T>(other));
}

template <typename T>
constexpr Vector4<T>& Vector4<T>::operator*=(T const& other)
{
    return *this = multiply(Vector4<T>(other));
}

template <typename T>
constexpr Vector4<T>& Vector4<T>::operator/=(T const& other)
{
    return *this = divide(Vector4<T>(other));
}

template <typename U>
static std::ostream& operator<<(std::ostream& stream, Vector4<U> const& vector)
{
    return stream << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w;
}

template <typename U, typename V>
constexpr static auto operator+(Vector4<U> const& lhs, Vector4<V> const& rhs)
{
    return lhs.add(rhs);
}

template <typename U, typename V>
constexpr static auto operator-(Vector4<U> const& lhs, Vector4<V> const& rhs)
{
    return lhs.subtract(rhs);
}

template <typename U, typename V>
constexpr static auto operator*(Vector4<U> const& lhs, Vector4<V> const& rhs)
{
    return lhs.multiply(rhs);
}

template <typename U, typename V>
constexpr static auto operator/(Vector4<U> const& lhs, Vector4<V> const& rhs)
{
    return lhs.divide(rhs);
}

template <typename U, typename V>
constexpr static auto operator+(U const& lhs, Vector4<V> const& rhs)
{
    return Vector4<U>(lhs).add(rhs);
}

template <typename U, typename V>
constexpr static auto operator-(U const& lhs, Vector4<V> const& rhs)
{
    return Vector4<U>(lhs).subtract(rhs);
}

template <typename U, typename V>
constexpr static auto operator*(U const& lhs, Vector4<V> const& rhs)
{
    return Vector4<U>(lhs).multiply(rhs);
}

template <typename U, typename V>
constexpr static auto operator/(U const& lhs, Vector4<V> const& rhs)
{
    return Vector4<U>(lhs).divide(rhs);
}

template <typename U, typename V>
constexpr static auto operator+(Vector4<U> const& lhs, V const& rhs)
{
    return lhs.add(Vector4<V>(rhs));
}

template <typename U, typename V>
constexpr static auto operator-(Vector4<U> const& lhs, V const& rhs)
{
    return lhs.subtract(Vector4<V>(rhs));
}

template <typename U, typename V>
constexpr static auto operator*(Vector4<U> const& lhs, V const& rhs)
{
    return lhs.multiply(Vector4<V>(rhs));
}

template <typename U, typename V>
constexpr static auto operator/(Vector4<U> const& lhs, V const& rhs)
{
    return lhs.divide(Vector4<V>(rhs));
}

template <Integral U, Integral V>
constexpr static Vector4<V> operator&(Vector4<U> const& lhs, Vector4<V> const& rhs)
{
    return {lhs.x & rhs.x, lhs.y & rhs.y, lhs.z & rhs.z, lhs.w & rhs.w};
}

template <Integral U, Integral V>
constexpr static Vector4<V> operator|(Vector4<U> const& lhs, Vector4<V> const& rhs)
{
    return {lhs.x | rhs.x, lhs.y | rhs.y, lhs.z | rhs.z, lhs.w | rhs.w};
}

template <Integral U, Integral V>
constexpr static Vector4<V> operator>>(Vector4<U> const& lhs, Vector4<V> const& rhs)
{
    return {lhs.x >> rhs.x, lhs.y >> rhs.y, lhs.z >> rhs.z, lhs.w >> rhs.w};
}

template <Integral U, Integral V>
constexpr static Vector4<V> operator<<(Vector4<U> const& lhs, Vector4<V> const& rhs)
{
    return {lhs.x << rhs.x, lhs.y << rhs.y, lhs.z << rhs.z, lhs.w << rhs.w};
}

template <Integral U, Integral V>
constexpr static Vector4<V> operator&(Vector4<U> const& lhs, V const& rhs)
{
    return {lhs.x & rhs, lhs.y & rhs, lhs.z & rhs, lhs.w & rhs};
}

template <Integral U, Integral V>
constexpr static Vector4<V> operator|(Vector4<U> const& lhs, V const& rhs)
{
    return {lhs.x | rhs, lhs.y | rhs, lhs.z | rhs, lhs.w | rhs};
}

template <Integral U, Integral V>
constexpr static Vector4<V> operator>>(Vector4<U> const& lhs, V const& rhs)
{
    return {lhs.x >> rhs, lhs.y >> rhs, lhs.z >> rhs, lhs.w >> rhs};
}

template <Integral U, Integral V>
constexpr static Vector4<V> operator<<(Vector4<U> const& lhs, V const& rhs)
{
    return {lhs.x << rhs, lhs.y << rhs, lhs.z << rhs, lhs.w << rhs};
}
}