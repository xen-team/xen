#pragma once

#include <cstdint>

namespace xen {
template<typename T>
class Vector2 {
public:
    T x = 0;
    T y = 0;

	static const Vector2 zero;
	static const Vector2 one;
	static const Vector2 infinity;
	static const Vector2 left;
	static const Vector2 right;
	static const Vector2 up;
	static const Vector2 down;
    
public:
    constexpr Vector2() = default;

    constexpr explicit Vector2(const T& a);

    constexpr Vector2(const T& a, const T& b);

	template<typename U>
	constexpr Vector2(const Vector2<U> &src);

    constexpr Vector2(const Vector2<T>& src);

    constexpr Vector2(Vector2<T>&& src) noexcept;

    template<typename K>
	constexpr auto add(const Vector2<K> &other) const;

	template<typename K>
	constexpr auto subtract(const Vector2<K> &other) const;

	template<typename K>
	constexpr auto multiply(const Vector2<K> &other) const;

	template<typename K>
	constexpr auto divide(const Vector2<K> &other) const;

	template<typename K>
	constexpr auto angle(const Vector2<K> &other) const;

	template<typename K>
	constexpr auto dot(const Vector2<K> &other) const;

	template<typename K, typename J = float>
	constexpr auto lerp(const Vector2<K> &other, const J &progression) const;

	template<typename K = float>
	constexpr auto scale(const K &scalar) const;

	template<typename K = float>
	auto rotate(const K &angle) const;

	template<typename K = float, typename J>
	auto rotate(const K &angle, const Vector2<J> &rotationAxis) const;

	auto normalize() const;

	auto length() const;

	constexpr auto length_squared() const;

	auto abs() const;

	constexpr auto min() const;

	constexpr auto max() const;

	constexpr auto min_max() const;

	template<typename K>
	constexpr auto min(const Vector2<K> &other);

	template<typename K>
	constexpr auto max(const Vector2<K> &other);

	template<typename K>
	auto distance(const Vector2<K> &other) const;

	template<typename K>
	constexpr auto distance_squared(const Vector2<K> &other) const;

	template<typename K>
	constexpr auto distance_vector(const Vector2<K> &other) const;

	template<typename K>
	constexpr bool in_triangle(const Vector2<K> &v1, const Vector2<K> &v2, const Vector2<K> &v3) const;

	auto cartesian_to_polar() const;

	auto polar_to_cartesian() const;

	constexpr const T &operator[](uint32_t index) const;
	constexpr T &operator[](uint32_t index);

	constexpr void operator=(const Vector2<T>& other);

    constexpr void operator=(Vector2<T>&& other) noexcept;

    template<typename U>
    constexpr void operator=(const Vector2<U>& other);

	template<typename U>
	constexpr bool operator==(const Vector2<U>& other) const;
	template<typename U>
	constexpr bool operator!=(const Vector2<U>& other) const;

    template<typename U = T>
	constexpr auto operator-() const -> std::enable_if_t<std::is_signed_v<U>, Vector2>;

    template<typename U = T>
	constexpr auto operator~() const -> std::enable_if_t<std::is_integral_v<U>, Vector2>;

	template<typename U>
	constexpr Vector2 &operator+=(const Vector2<U>& other);
	template<typename U>
	constexpr Vector2 &operator-=(const Vector2<U>& other);
	template<typename U>
	constexpr Vector2 &operator*=(const Vector2<U>& other);
	template<typename U>
	constexpr Vector2 &operator/=(const Vector2<U>& other);
	constexpr Vector2 &operator+=(const T& other);
	constexpr Vector2 &operator-=(const T& other);
	constexpr Vector2 &operator*=(const T& other);
	constexpr Vector2 &operator/=(const T& other);
};

using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector2i = Vector2<int32_t>;
using Vector2ui = Vector2<uint32_t>;
using Vector2us = Vector2<uint16_t>;
}

#include "vector2.inl"