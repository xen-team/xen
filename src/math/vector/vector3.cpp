#include "vector3.hpp"

namespace xen {
template <>
Vector3f const Vector3f::Zero(0.f);
template <>
Vector3f const Vector3f::One(1.f);
template <>
Vector3f const Vector3f::Infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector3f const Vector3f::Left(-1.f, 0.f, 0.f);
template <>
Vector3f const Vector3f::Right(1.f, 0.f, 0.f);
template <>
Vector3f const Vector3f::Up(0.f, 1.f, 0.f);
template <>
Vector3f const Vector3f::Down(0.f, -1.f, 0.f);
template <>
Vector3f const Vector3f::Front(0.f, 0.f, 1.f);
template <>
Vector3f const Vector3f::Back(0.f, 0.f, -1.f);

template <>
Vector3d const Vector3d::Zero(0.0);
template <>
Vector3d const Vector3d::One(1.0);
template <>
Vector3d const Vector3d::Infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector3d const Vector3d::Left(-1.0, 0.0, 0.0);
template <>
Vector3d const Vector3d::Right(1.0, 0.0, 0.0);
template <>
Vector3d const Vector3d::Up(0.0, 1.0, 0.0);
template <>
Vector3d const Vector3d::Down(0.0, -1.0, 0.0);
template <>
Vector3d const Vector3d::Front(0.0, 0.0, 1.0);
template <>
Vector3d const Vector3d::Back(0.0, 0.0, -1.0);

template <>
Vector3i const Vector3i::Zero(0);
template <>
Vector3i const Vector3i::One(1);
template <>
Vector3i const Vector3i::Infinity(std::numeric_limits<value_type>::infinity());
template <>
Vector3i const Vector3i::Left(-1, 0, 0);
template <>
Vector3i const Vector3i::Right(1, 0, 0);
template <>
Vector3i const Vector3i::Up(0, 1, 0);
template <>
Vector3i const Vector3i::Down(0, -1, 0);
template <>
Vector3i const Vector3i::Front(0, 0, 1);
template <>
Vector3i const Vector3i::Back(0, 0, -1);

template <>
Vector3ui const Vector3ui::Zero(0);
template <>
Vector3ui const Vector3ui::One(1);
template <>
Vector3ui const Vector3ui::Infinity(std::numeric_limits<value_type>::infinity());
}