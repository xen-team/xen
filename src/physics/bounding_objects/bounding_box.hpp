#pragma once

#include <utility>

#include "aabb.hpp"

#include "math/vector3.hpp"
#include "math/quaternion.hpp"

namespace xen {
class BoundingBox {
public:
    Vector3f center{0.0f, 0.0f, 0.0f};
    Vector3f min = Vector3f(0.0f);
    Vector3f max = Vector3f(0.0f);
    Quaternion rotation{1.0f, 0.0f, 0.0f, 0.0f};

    constexpr BoundingBox() = default;

    constexpr BoundingBox(Vector3f center, Vector3f const& halfSize) :
        center(std::move(center)), min(-halfSize), max(halfSize)
    {
    }

    [[nodiscard]] constexpr Vector3f length() const { return max - min; }

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(center, min, max, rotation);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(center, min, max, rotation);
    }
};

constexpr BoundingBox ToBoundingBox(const AABB& aabb)
{
    BoundingBox result;
    result.max = aabb.max;
    result.min = aabb.min;
    return result;
}

constexpr bool operator==(BoundingBox const& b1, BoundingBox const& b2)
{
    return b1.center == b2.center && b1.max == b2.max && b1.min == b2.min && b1.rotation == b2.rotation;
}

constexpr bool operator!=(BoundingBox const& b1, BoundingBox const& b2)
{
    return !(b1 == b2);
}
}