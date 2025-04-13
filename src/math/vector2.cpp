#include "vector2.hpp"

#include <limits>

namespace xen {
template <>
Vector2f const Vector2f::zero(0.0f);
template <>
Vector2f const Vector2f::one(1.0f);
template <>
Vector2f const Vector2f::infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector2f const Vector2f::left(-1.0f, 0.0f);
template <>
Vector2f const Vector2f::right(1.0f, 0.0f);
template <>
Vector2f const Vector2f::up(0.0f, 1.0f);
template <>
Vector2f const Vector2f::down(0.0f, -1.0f);

template <>
Vector2d const Vector2d::zero(0.0);
template <>
Vector2d const Vector2d::one(1.0);
template <>
Vector2d const Vector2d::infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector2d const Vector2d::left(-1.0, 0.0);
template <>
Vector2d const Vector2d::right(1.0, 0.0);
template <>
Vector2d const Vector2d::up(0.0, 1.0);
template <>
Vector2d const Vector2d::down(0.0, -1.0);

template <>
Vector2i const Vector2i::zero(0);
template <>
Vector2i const Vector2i::one(1);
template <>
Vector2i const Vector2i::infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector2i const Vector2i::left(-1, 0);
template <>
Vector2i const Vector2i::right(1, 0);
template <>
Vector2i const Vector2i::up(0, 1);
template <>
Vector2i const Vector2i::down(0, -1);

template <>
Vector2ui const Vector2ui::zero(0);
template <>
Vector2ui const Vector2ui::one(1);
template <>
Vector2ui const Vector2ui::infinity(std::numeric_limits<value_type>::infinity());
}