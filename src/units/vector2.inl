#pragma once

#include "vector2.hpp"
#include "units/math.hpp"

namespace xen {
template<typename T>
constexpr Vector2<T>::Vector2(const T &a):
	x(a),
	y(a) {
}

template<typename T>
constexpr Vector2<T>::Vector2(const T &x, const T &y):
	x(x),
	y(y) {
}

template<typename T>
template<typename U>
constexpr Vector2<T>::Vector2(const Vector2<U>& src) :
	x(static_cast<T>(src.x)),
	y(static_cast<T>(src.y)) {
}

template<typename T>
constexpr Vector2<T>::Vector2(const Vector2<T>& src) :
	x(src.x),
	y(src.y) {
}

template<typename T>
constexpr Vector2<T>::Vector2(Vector2<T>&& src) noexcept:
	x(std::move(src.x)),
	y(std::move(src.y)) {
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::add(const Vector2<U> &other) const {
	return Vector2<decltype(x + other.x)>(x + other.x, y + other.y);
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::subtract(const Vector2<U> &other) const {
	return Vector2<decltype(x - other.x)>(x - other.x, y - other.y);
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::multiply(const Vector2<U> &other) const {
	return Vector2<decltype(x * other.x)>(x * other.x, y * other.y);
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::divide(const Vector2<U> &other) const {
	return Vector2<decltype(x / other.x)>(x / other.x, y / other.y);
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::angle(const Vector2<U> &other) const {
	auto dls = dot(other) / (length() * other.length());

	if (dls < -1) {
		dls = -1;
	} else if (dls > 1) {
		dls = 1;
	}

	return std::acos(dls);
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::dot(const Vector2<U> &other) const {
	return x * other.x + y * other.y;
}

template<typename T>
template<typename U, typename V>
constexpr auto Vector2<T>::lerp(const Vector2<U> &other, const V &progression) const {
	auto ta = *this * (1 - progression);
	auto tb = other * progression;
	return ta + tb;
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::scale(const U &scalar) const {
	return Vector2<decltype(x * scalar)>(x * scalar, y * scalar);
}

template<typename T>
template<typename U>
auto Vector2<T>::rotate(const U &angle) const {
	return Vector2<decltype(x * angle)>(x * std::cos(angle) - y * std::sin(angle), x * std::sin(angle) + y * std::cos(angle));
}

template<typename T>
template<typename U, typename V>
auto Vector2<T>::rotate(const U &angle, const Vector2<V> &rotationAxis) const {
	auto x1 = ((x - rotationAxis.x) * std::cos(angle)) - ((y - rotationAxis.y) * std::sin(angle) + rotationAxis.x);
	auto y1 = ((x - rotationAxis.x) * std::sin(angle)) + ((y - rotationAxis.y) * std::cos(angle) + rotationAxis.y);
	return Vector2<decltype(x1)>(x1, y1);
}

template<typename T>
auto Vector2<T>::normalize() const {
	auto const l = length();

	if (l == 0) {
		throw std::runtime_error("Can't normalize a zero length vector");
	}

	return *this / l;
}

template<typename T>
constexpr auto Vector2<T>::length_squared() const {
	return x * x + y * y;
}

template<typename T>
auto Vector2<T>::length() const {
	return std::sqrt(length_squared());
}

template<typename T>
auto Vector2<T>::abs() const {
	return Vector2<T>(std::abs(x), std::abs(y));
}

template<typename T>
constexpr auto Vector2<T>::min() const {
	return std::min({x, y});
}

template<typename T>
constexpr auto Vector2<T>::max() const {
	return std::max({x, y});
}

template<typename T>
constexpr auto Vector2<T>::min_max() const {
	return std::minmax({x, y});
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::min(const Vector2<U> &other) {
	using type = decltype(x + other.x);
	return Vector2<type>(std::min<type>(x, other.x), std::min<type>(y, other.y));
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::max(const Vector2<U> &other) {
	using type = decltype(x + other.x);
	return Vector2<type>(std::max<type>(x, other.x), std::max<type>(y, other.y));
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::distance_squared(const Vector2<U> &other) const {
	auto const dx = x - other.x;
	auto const dy = y - other.y;
	return dx * dx + dy * dy;
}

template<typename T>
template<typename U>
auto Vector2<T>::distance(const Vector2<U> &other) const {
	return std::sqrt(distance_squared(other));
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::distance_vector(const Vector2<U> &other) const {
	return (*this - other) * (*this - other);
}

template<typename T>
template<typename U>
constexpr bool Vector2<T>::in_triangle(const Vector2<U> &v1, const Vector2<U> &v2, const Vector2<U> &v3) const {
	auto const b1 = ((x - v2.x) * (v1.y - v2.y) - (v1.x - v2.x) * (y - v2.y)) < 0;
	auto const b2 = ((x - v3.x) * (v2.y - v3.y) - (v2.x - v3.x) * (y - v3.y)) < 0;
	auto const b3 = ((x - v1.x) * (v3.y - v1.y) - (v3.x - v1.x) * (y - v1.y)) < 0;
	return ((b1 == b2) & (b2 == b3));
}

template<typename T>
auto Vector2<T>::cartesian_to_polar() const {
	auto const radius = std::sqrt(x * x + y * y);
	auto const theta = std::atan2(y, x);
	return Vector2<decltype(radius)>(radius, theta);
}

template<typename T>
auto Vector2<T>::polar_to_cartesian() const {
	auto const x1 = x * std::cos(y);
	auto const y1 = x * std::sin(y);
	return Vector2<decltype(x1)>(x1, y1);
}

template<typename T>
constexpr void Vector2<T>::operator=(const Vector2<T>& other) {
    this->x = other.x;
    this->y = other.y;
}

template<typename T>
constexpr void Vector2<T>::operator=(Vector2<T>&& other) noexcept {
    this->x = std::move(other.x);
    this->y = std::move(other.y);
}

template<typename T>
template<typename U>
constexpr void Vector2<T>::operator=(const Vector2<U>& other) {
    this->x = static_cast<T>(other.x);
    this->y = static_cast<T>(other.y);
}

template<typename T>
constexpr const T &Vector2<T>::operator[](uint32_t index) const {
    if (index == 0) {
        return x;
    } else if (index == 1) {
        return y;
    } else {
        throw std::runtime_error("Vector2 index out of bounds!");
    }
}

template<typename T>
constexpr T &Vector2<T>::operator[](uint32_t index) {
    if (index == 0) {
        return x;
    } else if (index == 1) {
        return y;
    } else {
        throw std::runtime_error("Vector2 index out of bounds!");
    }
}

template<typename T>
template<typename U>
constexpr bool Vector2<T>::operator==(const Vector2<U> &other) const {
	return x == other.x && y == other.y;
}

template<typename T>
template<typename U>
constexpr bool Vector2<T>::operator!=(const Vector2<U> &other) const {
	return !operator==(other);
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::operator-() const -> std::enable_if_t<std::is_signed_v<U>, Vector2<T>> {
	return {-x, -y};
}

template<typename T>
template<typename U>
constexpr auto Vector2<T>::operator~() const -> std::enable_if_t<std::is_integral_v<U>, Vector2<T>> {
	return {~x, ~y};
}

template<typename T>
template<typename U>
constexpr Vector2<T> &Vector2<T>::operator+=(const Vector2<U> &other) {
	return *this = add(other);
}

template<typename T>
template<typename U>
constexpr Vector2<T> &Vector2<T>::operator-=(const Vector2<U> &other) {
	return *this = subtract(other);
}

template<typename T>
template<typename U>
constexpr Vector2<T> &Vector2<T>::operator*=(const Vector2<U> &other) {
	return *this = multiply(other);
}

template<typename T>
template<typename U>
constexpr Vector2<T> &Vector2<T>::operator/=(const Vector2<U> &other) {
	return *this = divide(other);
}

template<typename T>
constexpr Vector2<T> &Vector2<T>::operator+=(const T &other) {
	return *this = add(Vector2<T>(other));
}

template<typename T>
constexpr Vector2<T> &Vector2<T>::operator-=(const T &other) {
	return *this = subtract(Vector2<T>(other));
}

template<typename T>
constexpr Vector2<T> &Vector2<T>::operator*=(const T &other) {
	return *this = multiply(Vector2<T>(other));
}

template<typename T>
constexpr Vector2<T> &Vector2<T>::operator/=(const T &other) {
	return *this = divide(Vector2<T>(other));
}

template<typename U>
std::ostream &operator<<(std::ostream &stream, const Vector2<U> &vector) {
	return stream << vector.x << ", " << vector.y;
}

template<typename U, typename V>
constexpr auto operator+(const Vector2<U> &lhs, const Vector2<V> &rhs) {
	return lhs.add(rhs);
}

template<typename U, typename V>
constexpr auto operator-(const Vector2<U> &lhs, const Vector2<V> &rhs) {
	return lhs.subtract(rhs);
}

template<typename U, typename V>
constexpr auto operator*(const Vector2<U> &lhs, const Vector2<V> &rhs) {
	return lhs.multiply(rhs);
}

template<typename U, typename V>
constexpr auto operator/(const Vector2<U> &lhs, const Vector2<V> &rhs) {
	return lhs.divide(rhs);
}

template<typename U, typename V>
constexpr auto operator+(const U &lhs, const Vector2<V> &rhs) {
	return Vector2<U>(lhs).add(rhs);
}

template<typename U, typename V>
constexpr auto operator-(const U &lhs, const Vector2<V> &rhs) {
	return Vector2<U>(lhs).subtract(rhs);
}

template<typename U, typename V>
constexpr auto operator*(const U &lhs, const Vector2<V> &rhs) {
	return Vector2<U>(lhs).multiply(rhs);
}

template<typename U, typename V>
constexpr auto operator/(const U &lhs, const Vector2<V> &rhs) {
	return Vector2<U>(lhs).divide(rhs);
}

template<typename U, typename V>
constexpr auto operator+(const Vector2<U> &lhs, const V &rhs) {
	return lhs.add(Vector2<V>(rhs));
}

template<typename U, typename V>
constexpr auto operator-(const Vector2<U> &lhs, const V &rhs) {
	return lhs.subtract(Vector2<V>(rhs));
}

template<typename U, typename V>
constexpr auto operator*(const Vector2<U> &lhs, const V &rhs) {
	return lhs.multiply(Vector2<V>(rhs));
}

template<typename U, typename V>
constexpr auto operator/(const Vector2<U> &lhs, const V &rhs) {
	return lhs.divide(Vector2<V>(rhs));
}

template<typename U, typename V>
constexpr auto operator&(const Vector2<U> &lhs, const Vector2<V> &rhs) -> std::enable_if_t<std::is_integral_v<U> &&std::is_integral_v<V>, Vector2<V>> {
	return {lhs.x & rhs.x, lhs.y & rhs.y};
}

template<typename U, typename V>
constexpr auto operator|(const Vector2<U> &lhs, const Vector2<V> &rhs) -> std::enable_if_t<std::is_integral_v<U> &&std::is_integral_v<V>, Vector2<V>> {
	return {lhs.x | rhs.x, lhs.y | rhs.y};
}

template<typename U, typename V>
constexpr auto operator>>(const Vector2<U> &lhs, const Vector2<V> &rhs) -> std::enable_if_t<std::is_integral_v<U> &&std::is_integral_v<V>, Vector2<V>> {
	return {lhs.x >> rhs.x, lhs.y >> rhs.y};
}

template<typename U, typename V>
constexpr auto operator<<(const Vector2<U> &lhs, const Vector2<V> &rhs) -> std::enable_if_t<std::is_integral_v<U> &&std::is_integral_v<V>, Vector2<V>> {
	return {lhs.x << rhs.x, lhs.y << rhs.y};
}

template<typename U, typename V>
constexpr auto operator&(const Vector2<U> &lhs, const V &rhs) -> std::enable_if_t<std::is_integral_v<U> &&std::is_integral_v<V>, Vector2<V>> {
	return {lhs.x & rhs, lhs.y & rhs};
}

template<typename U, typename V>
constexpr auto operator|(const Vector2<U> &lhs, const V &rhs) -> std::enable_if_t<std::is_integral_v<U> &&std::is_integral_v<V>, Vector2<V>> {
	return {lhs.x | rhs, lhs.y | rhs};
}

template<typename U, typename V>
constexpr auto operator>>(const Vector2<U> &lhs, const V &rhs) -> std::enable_if_t<std::is_integral_v<U> &&std::is_integral_v<V>, Vector2<V>> {
	return {lhs.x >> rhs, lhs.y >> rhs};
}

template<typename U, typename V>
constexpr auto operator<<(const Vector2<U> &lhs, const V &rhs) -> std::enable_if_t<std::is_integral_v<U> &&std::is_integral_v<V>, Vector2<V>> {
	return {lhs.x << rhs, lhs.y << rhs};
}
}