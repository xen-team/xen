#include "vector2.hpp"
#include <limits>

namespace xen {
template<>
const Vector2f Vector2f::zero(0.0f);
template<>
const Vector2f Vector2f::one(1.0f);
template<>
const Vector2f Vector2f::infinity(std::numeric_limits<float>::infinity());
template<>
const Vector2f Vector2f::left(-1.0f, 0.0f);
template<>
const Vector2f Vector2f::right(1.0f, 0.0f);
template<>
const Vector2f Vector2f::up(0.0f, 1.0f);
template<>
const Vector2f Vector2f::down(0.0f, -1.0f);

template<>
const Vector2d Vector2d::zero(0.0);
template<>
const Vector2d Vector2d::one(1.0);
template<>
const Vector2d Vector2d::infinity(std::numeric_limits<double>::infinity());
template<>
const Vector2d Vector2d::left(-1.0, 0.0);
template<>
const Vector2d Vector2d::right(1.0, 0.0);
template<>
const Vector2d Vector2d::up(0.0, 1.0);
template<>
const Vector2d Vector2d::down(0.0, -1.0);

template<>
const Vector2i Vector2i::zero(0);
template<>
const Vector2i Vector2i::one(1);
template<>
const Vector2i Vector2i::infinity(std::numeric_limits<int32_t>::infinity());
template<>
const Vector2i Vector2i::left(-1, 0);
template<>
const Vector2i Vector2i::right(1, 0);
template<>
const Vector2i Vector2i::up(0, 1);
template<>
const Vector2i Vector2i::down(0, -1);

template<>
const Vector2ui Vector2ui::zero(0);
template<>
const Vector2ui Vector2ui::one(1);
}