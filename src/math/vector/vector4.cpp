#include "vector4.hpp"

namespace xen {
template <>
Vector4f const Vector4f::Zero(0.f);
template <>
Vector4f const Vector4f::One(1.f);
template <>
Vector4f const Vector4f::Infinity(std::numeric_limits<value_type>::infinity());

template <>
Vector4d const Vector4d::Zero(0.0);
template <>
Vector4d const Vector4d::One(1.0);
template <>
Vector4d const Vector4d::Infinity(std::numeric_limits<value_type>::infinity());

template <>
Vector4i const Vector4i::Zero(0);
template <>
Vector4i const Vector4i::One(1);
template <>
Vector4i const Vector4i::Infinity(std::numeric_limits<value_type>::infinity());

template <>
Vector4ui const Vector4ui::Zero(0);
template <>
Vector4ui const Vector4ui::One(1);
template <>
Vector4ui const Vector4ui::Infinity(std::numeric_limits<value_type>::infinity());
}