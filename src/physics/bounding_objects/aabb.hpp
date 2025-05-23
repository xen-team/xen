#pragma once

namespace xen {
class AABB {
public:
    Vector3f min;
    Vector3f max;

public:
    [[nodiscard]] constexpr Vector3f length() const { return max - min; }

    [[nodiscard]] constexpr Vector3f get_center() const { return (max + min) * 0.5f; }
};

constexpr AABB operator*(const AABB& box, Vector3f const& scale)
{
    return {.min = box.min * scale, .max = box.max * scale};
}

constexpr AABB operator/(const AABB& box, Vector3f const& scale)
{
    return {.min = box.min / scale, .max = box.max / scale};
}

constexpr AABB operator+(const AABB& box, Vector3f const& translate)
{
    return {.min = box.min + translate, .max = box.max + translate};
}

constexpr AABB operator-(const AABB& box, Vector3f const& translate)
{
    return {.min = box.min - translate, .max = box.max - translate};
}

constexpr bool operator==(const AABB& box1, const AABB& box2)
{
    constexpr auto abs = [](float x) { return x >= 0.0f ? x : -x; };

    auto const d1 = box1.min - box2.min;
    auto const d2 = box1.max - box2.max;
    bool const res1 = abs(d1.x) < 0.01f && abs(d1.y) < 0.01f && abs(d1.z) < 0.01f;
    bool const res2 = abs(d2.x) < 0.01f && abs(d2.y) < 0.01f && abs(d2.z) < 0.01f;
    return res1 && res2;
}

constexpr bool operator!=(const AABB& box1, const AABB& box2)
{
    return !(box1 == box2);
}
}