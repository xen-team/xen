#include "vector4.hpp"

#include <limits>

namespace xen {
template <>
Vector4f const Vector4f::zero(0.f);
template <>
Vector4f const Vector4f::one(1.f);
template <>
Vector4f const Vector4f::infinity(std::numeric_limits<value_type>::infinity());

template <>
Vector4d const Vector4d::zero(0.0);
template <>
Vector4d const Vector4d::one(1.0);
template <>
Vector4d const Vector4d::infinity(std::numeric_limits<value_type>::infinity());

template <>
Vector4i const Vector4i::zero(0);
template <>
Vector4i const Vector4i::one(1);
template <>
Vector4i const Vector4i::infinity(std::numeric_limits<value_type>::infinity());

template <>
Vector4ui const Vector4ui::zero(0);
template <>
Vector4ui const Vector4ui::one(1);
template <>
Vector4ui const Vector4ui::infinity(std::numeric_limits<value_type>::infinity());
}