#include "vector3.hpp"

#include <limits>

namespace xen {
template <>
Vector3f const Vector3f::zero(0.f);
template <>
Vector3f const Vector3f::one(1.f);
template <>
Vector3f const Vector3f::infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector3f const Vector3f::left(-1.f, 0.f, 0.f);
template <>
Vector3f const Vector3f::right(1.f, 0.f, 0.f);
template <>
Vector3f const Vector3f::up(0.f, 1.f, 0.f);
template <>
Vector3f const Vector3f::down(0.f, -1.f, 0.f);
template <>
Vector3f const Vector3f::front(0.f, 0.f, 1.f);
template <>
Vector3f const Vector3f::back(0.f, 0.f, -1.f);

template <>
Vector3d const Vector3d::zero(0.0);
template <>
Vector3d const Vector3d::one(1.0);
template <>
Vector3d const Vector3d::infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector3d const Vector3d::left(-1.0, 0.0, 0.0);
template <>
Vector3d const Vector3d::right(1.0, 0.0, 0.0);
template <>
Vector3d const Vector3d::up(0.0, 1.0, 0.0);
template <>
Vector3d const Vector3d::down(0.0, -1.0, 0.0);
template <>
Vector3d const Vector3d::front(0.0, 0.0, 1.0);
template <>
Vector3d const Vector3d::back(0.0, 0.0, -1.0);

template <>
Vector3i const Vector3i::zero(0);
template <>
Vector3i const Vector3i::one(1);
template <>
Vector3i const Vector3i::infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector3i const Vector3i::left(-1, 0, 0);
template <>
Vector3i const Vector3i::right(1, 0, 0);
template <>
Vector3i const Vector3i::up(0, 1, 0);
template <>
Vector3i const Vector3i::down(0, -1, 0);
template <>
Vector3i const Vector3i::front(0, 0, 1);
template <>
Vector3i const Vector3i::back(0, 0, -1);

template <>
Vector3ui const Vector3ui::zero(0);
template <>
Vector3ui const Vector3ui::one(1);
template <>
Vector3ui const Vector3ui::infinity(std::numeric_limits<value_type>::infinity());
}