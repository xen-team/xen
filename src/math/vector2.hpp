#pragma once

#include "core.hpp"

#include <cstdint>

namespace xen {
template <typename T>
class Vector3;

template <typename T>
class Vector4;

template <typename T>
class XEN_API Vector2 {
public:
    using value_type = T;

    T x = 0;
    T y = 0;

    static Vector2 const zero;
    static Vector2 const one;
    static Vector2 const infinity;
    static Vector2 const left;
    static Vector2 const right;
    static Vector2 const up;
    static Vector2 const down;

public:
    constexpr Vector2() = default;

    constexpr explicit Vector2(T const& a);

    constexpr Vector2(T const& x, T const& y);

    template <typename K, typename J>
    constexpr Vector2(K const& x, J const& y);

    template <typename U>
    constexpr Vector2(Vector3<U> const& src);

    template <typename U>
    constexpr Vector2(Vector4<U> const& src);

    template <typename U>
    constexpr Vector2(Vector2<U> const& src);

    constexpr Vector2(Vector2<T> const& src);

    constexpr Vector2(Vector2<T>&& src) noexcept;

    ~Vector2() = default;

    template <typename K>
    [[nodiscard]] constexpr auto add(Vector2<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto subtract(Vector2<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto multiply(Vector2<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto divide(Vector2<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto angle(Vector2<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto dot(Vector2<K> const& other) const;

    template <typename K, typename J = float>
    [[nodiscard]] constexpr auto lerp(Vector2<K> const& other, J const& progression) const;

    template <typename K = float>
    [[nodiscard]] constexpr auto scale(K const& scalar) const;

    template <typename K = float>
    [[nodiscard]] constexpr auto rotate(K const& angle) const;

    template <typename K = float, typename J>
    [[nodiscard]] constexpr auto rotate(K const& angle, Vector2<J> const& rotation_axis) const;

    [[nodiscard]] constexpr auto normalize() const;

    [[nodiscard]] constexpr auto length() const;

    [[nodiscard]] constexpr auto length_squared() const;

    [[nodiscard]] constexpr auto abs() const;

    [[nodiscard]] constexpr auto min() const;

    [[nodiscard]] constexpr auto max() const;

    [[nodiscard]] constexpr auto min_max() const;

    template <typename K>
    [[nodiscard]] constexpr auto min(Vector2<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto max(Vector2<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto distance(Vector2<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto distance_squared(Vector2<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto distance_vector(Vector2<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr bool in_triangle(Vector2<K> const& v1, Vector2<K> const& v2, Vector2<K> const& v3) const;

    [[nodiscard]] constexpr auto cartesian_to_polar() const;

    [[nodiscard]] constexpr auto polar_to_cartesian() const;

    [[nodiscard]] constexpr T const& operator[](uint32_t index) const;
    [[nodiscard]] constexpr T& operator[](uint32_t index);

    auto operator<=>(Vector2<T> const&) const = default;

    constexpr Vector2<T>& operator=(Vector2<T> const& other);

    constexpr Vector2<T>& operator=(Vector2<T>&& other) noexcept;

    template <typename U>
    constexpr Vector2<T>& operator=(Vector2<U> const& other);

    template <typename U>
    constexpr bool operator==(Vector2<U> const& other) const;
    template <typename U>
    constexpr bool operator!=(Vector2<U> const& other) const;

    template <typename U = T>
    constexpr auto operator-() const -> std::enable_if_t<std::is_signed_v<U>, Vector2>;

    template <typename U = T>
    constexpr auto operator~() const -> std::enable_if_t<std::is_integral_v<U>, Vector2>;

    template <typename U>
    constexpr Vector2& operator+=(Vector2<U> const& other);
    template <typename U>
    constexpr Vector2& operator-=(Vector2<U> const& other);
    template <typename U>
    constexpr Vector2& operator*=(Vector2<U> const& other);
    template <typename U>
    constexpr Vector2& operator/=(Vector2<U> const& other);
    constexpr Vector2& operator+=(T const& other);
    constexpr Vector2& operator-=(T const& other);
    constexpr Vector2& operator*=(T const& other);
    constexpr Vector2& operator/=(T const& other);

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(x, y);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(x, y);
    }
};

using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector2i = Vector2<int32_t>;
using Vector2ui = Vector2<uint32_t>;
using Vector2us = Vector2<uint16_t>;
}

#include "vector2.inl"