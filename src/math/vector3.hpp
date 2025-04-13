#pragma once

#include "core.hpp"

#include <cstdint>

namespace xen {
template <typename T>
class Vector2;

template <typename T>
class Vector4;

template <typename T>
class XEN_API Vector3 {
public:
    using value_type = T;

    T x = 0;
    T y = 0;
    T z = 0;

    static Vector3 const zero;
    static Vector3 const one;
    static Vector3 const infinity;
    static Vector3 const left;
    static Vector3 const right;
    static Vector3 const up;
    static Vector3 const down;
    static Vector3 const front;
    static Vector3 const back;

public:
    constexpr Vector3() = default;

    constexpr explicit Vector3(T const& a);

    constexpr Vector3(T const& x, T const& y, T const& z);

    template <typename K, typename J, typename H>
    constexpr Vector3(K const& x, J const& y, H const& z);

    template <typename K>
    constexpr Vector3(Vector2<K> const& src);

    template <typename K, typename J = T>
    constexpr explicit Vector3(Vector2<K> const& src, J const& z = 0);

    template <typename U>
    constexpr Vector3(Vector4<U> const& src);

    template <typename U>
    constexpr Vector3(Vector3<U> const& src);

    constexpr Vector3(Vector3<T> const& src);

    constexpr Vector3(Vector3<T>&& src) noexcept;

    ~Vector3() = default;

    template <typename K>
    [[nodiscard]] constexpr auto add(Vector3<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto subtract(Vector3<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto multiply(Vector3<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto divide(Vector3<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto angle(Vector3<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto dot(Vector3<K> const& other) const;

    template <typename K>
    constexpr auto cross(Vector3<K> const& other) const;

    template <typename K, typename J = float>
    [[nodiscard]] constexpr auto lerp(Vector3<K> const& other, J const& progression) const;

    template <typename K = float>
    [[nodiscard]] constexpr auto scale(K const& scalar) const;

    // template <typename K = float>
    // [[nodiscard]] constexpr auto rotate(K const& angle) const;

    [[nodiscard]] constexpr auto normalize() const;

    [[nodiscard]] constexpr auto length() const;

    [[nodiscard]] constexpr auto length_squared() const;

    [[nodiscard]] constexpr auto abs() const;

    [[nodiscard]] constexpr auto min() const;

    [[nodiscard]] constexpr auto max() const;

    [[nodiscard]] constexpr auto min_max() const;

    template <typename K>
    [[nodiscard]] constexpr auto min(Vector3<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto max(Vector3<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto distance(Vector3<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto distance_squared(Vector3<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto distance_vector(Vector3<K> const& other) const;

    [[nodiscard]] constexpr auto cartesian_to_polar() const;

    [[nodiscard]] constexpr auto polar_to_cartesian() const;

    [[nodiscard]] constexpr T const& operator[](uint32_t index) const;
    [[nodiscard]] constexpr T& operator[](uint32_t index);

    auto operator<=>(Vector3<T> const&) const = default;

    constexpr Vector3<T>& operator=(Vector3<T> const& other);

    constexpr Vector3<T>& operator=(Vector3<T>&& other) noexcept;

    template <typename U>
    constexpr Vector3<T>& operator=(Vector3<U> const& other);

    template <typename U>
    constexpr bool operator==(Vector3<U> const& other) const;
    template <typename U>
    constexpr bool operator!=(Vector3<U> const& other) const;

    template <typename U = T>
    constexpr auto operator-() const -> std::enable_if_t<std::is_signed_v<U>, Vector3>;

    template <typename U = T>
    constexpr auto operator~() const -> std::enable_if_t<std::is_integral_v<U>, Vector3>;

    template <typename U>
    constexpr Vector3& operator+=(Vector3<U> const& other);
    template <typename U>
    constexpr Vector3& operator-=(Vector3<U> const& other);
    template <typename U>
    constexpr Vector3& operator*=(Vector3<U> const& other);
    template <typename U>
    constexpr Vector3& operator/=(Vector3<U> const& other);
    constexpr Vector3& operator+=(T const& other);
    constexpr Vector3& operator-=(T const& other);
    constexpr Vector3& operator*=(T const& other);
    constexpr Vector3& operator/=(T const& other);

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(x, y, z);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(x, y, z);
    }
};

using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;
using Vector3i = Vector3<int32_t>;
using Vector3ui = Vector3<uint32_t>;
using Vector3us = Vector3<uint16_t>;
}

#include "vector3.inl"