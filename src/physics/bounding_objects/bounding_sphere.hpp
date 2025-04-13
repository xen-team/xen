#pragma once

#include <utility>

#include "aabb.hpp"

#include "math/vector3.hpp"

namespace xen {
class BoundingSphere {
public:
    Vector3f center = Vector3f(0.f);
    float radius = 0.0f;

    constexpr BoundingSphere() = default;

    constexpr BoundingSphere(Vector3f center, float const r) : center(std::move(center)), radius(r) {}

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(center, radius);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(center, radius);
    }
};

constexpr BoundingSphere to_sphere(const AABB& box)
{
    Vector3f const center = box.get_center();
    Vector3f const length = box.length();
    float const radius = length.max() * 0.5f;
    return {center, radius};
}

constexpr bool operator==(BoundingSphere const& b1, BoundingSphere const& b2)
{
    return b1.center == b2.center && b1.radius == b2.radius;
}

constexpr bool operator!=(BoundingSphere const& b1, BoundingSphere const& b2)
{
    return !(b1 == b2);
}
}