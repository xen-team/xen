#include "vector2.hpp"

namespace xen {
template <>
Vector2f const Vector2f::Zero(0.0f);
template <>
Vector2f const Vector2f::One(1.0f);
template <>
Vector2f const Vector2f::Infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector2f const Vector2f::Left(-1.0f, 0.0f);
template <>
Vector2f const Vector2f::Right(1.0f, 0.0f);
template <>
Vector2f const Vector2f::Up(0.0f, 1.0f);
template <>
Vector2f const Vector2f::Down(0.0f, -1.0f);

template <>
Vector2d const Vector2d::Zero(0.0);
template <>
Vector2d const Vector2d::One(1.0);
template <>
Vector2d const Vector2d::Infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector2d const Vector2d::Left(-1.0, 0.0);
template <>
Vector2d const Vector2d::Right(1.0, 0.0);
template <>
Vector2d const Vector2d::Up(0.0, 1.0);
template <>
Vector2d const Vector2d::Down(0.0, -1.0);

template <>
Vector2i const Vector2i::Zero(0);
template <>
Vector2i const Vector2i::One(1);
template <>
Vector2i const Vector2i::Infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector2i const Vector2i::Left(-1, 0);
template <>
Vector2i const Vector2i::Right(1, 0);
template <>
Vector2i const Vector2i::Up(0, 1);
template <>
Vector2i const Vector2i::Down(0, -1);

template <>
Vector2ui const Vector2ui::Zero(0);
template <>
Vector2ui const Vector2ui::One(1);
template <>
Vector2ui const Vector2ui::Infinity(std::numeric_limits<value_type>::infinity());
}