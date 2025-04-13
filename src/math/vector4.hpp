#pragma once

#include "core.hpp"

#include <cstdint>

namespace xen {
template <typename T>
class Vector2;

template <typename T>
class Vector3;

template <typename T>
class XEN_API Vector4 {
public:
    using value_type = T;

    T x = 0;
    T y = 0;
    T z = 0;
    T w = 0;

    static Vector4 const zero;
    static Vector4 const one;
    static Vector4 const infinity;

public:
    constexpr Vector4() = default;

    constexpr explicit Vector4(T const& a);

    constexpr Vector4(T const& x, T const& y, T const& z, T const& w = 1);

    template <typename K, typename J, typename H, typename F>
    constexpr Vector4(K const& x, J const& y, H const& z, F const& w);

    template <typename K>
    constexpr Vector4(Vector2<K> const& src);

    template <typename K, typename J>
    constexpr Vector4(Vector2<K> const& left, Vector2<J> const& right = Vector2<K>::up);

    template <typename K, typename J = T>
    constexpr Vector4(Vector3<K> const& source, J const& w = 1);

    template <typename U>
    constexpr Vector4(Vector4<U> const& src);

    constexpr Vector4(Vector4<T> const& src);

    constexpr Vector4(Vector4<T>&& src) noexcept;

    ~Vector4() = default;

    template <typename K>
    [[nodiscard]] constexpr auto add(Vector4<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto subtract(Vector4<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto multiply(Vector4<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto divide(Vector4<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto angle(Vector4<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto dot(Vector4<K> const& other) const;

    template <typename K, typename J = float>
    [[nodiscard]] constexpr auto lerp(Vector4<K> const& other, J const& progression) const;

    template <typename K = float>
    [[nodiscard]] constexpr auto scale(K const& scalar) const;

    [[nodiscard]] constexpr auto normalize() const;

    [[nodiscard]] constexpr auto length() const;

    [[nodiscard]] constexpr auto length_squared() const;

    [[nodiscard]] constexpr auto abs() const;

    [[nodiscard]] constexpr auto min() const;

    [[nodiscard]] constexpr auto max() const;

    [[nodiscard]] constexpr auto min_max() const;

    template <typename K>
    [[nodiscard]] constexpr auto min(Vector4<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto max(Vector4<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto distance(Vector4<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto distance_squared(Vector4<K> const& other) const;

    template <typename K>
    [[nodiscard]] constexpr auto distance_vector(Vector4<K> const& other) const;

    [[nodiscard]] constexpr T const& operator[](uint32_t index) const;
    [[nodiscard]] constexpr T& operator[](uint32_t index);

    auto operator<=>(Vector4<T> const&) const = default;

    constexpr Vector4<T>& operator=(Vector4<T> const& other);

    constexpr Vector4<T>& operator=(Vector4<T>&& other) noexcept;

    template <typename U>
    constexpr Vector4<T>& operator=(Vector4<U> const& other);

    template <typename U>
    constexpr bool operator==(Vector4<U> const& other) const;
    template <typename U>
    constexpr bool operator!=(Vector4<U> const& other) const;

    template <typename U = T>
    constexpr auto operator-() const -> std::enable_if_t<std::is_signed_v<U>, Vector4>;

    template <typename U = T>
    constexpr auto operator~() const -> std::enable_if_t<std::is_integral_v<U>, Vector4>;

    template <typename U>
    constexpr Vector4& operator+=(Vector4<U> const& other);
    template <typename U>
    constexpr Vector4& operator-=(Vector4<U> const& other);
    template <typename U>
    constexpr Vector4& operator*=(Vector4<U> const& other);
    template <typename U>
    constexpr Vector4& operator/=(Vector4<U> const& other);
    constexpr Vector4& operator+=(T const& other);
    constexpr Vector4& operator-=(T const& other);
    constexpr Vector4& operator*=(T const& other);
    constexpr Vector4& operator/=(T const& other);

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(x, y, z, w);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(x, y, z, w);
    }
};

using Vector4f = Vector4<float>;
using Vector4d = Vector4<double>;
using Vector4i = Vector4<int32_t>;
using Vector4ui = Vector4<uint32_t>;
using Vector4us = Vector4<uint16_t>;
}

#include "vector4.inl"