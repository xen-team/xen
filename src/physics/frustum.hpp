#pragma once

#include <core.hpp>

#include "math/matrix4.hpp"

#include <array>

namespace xen {
class XEN_API Frustum {
private:
    std::array<std::array<float, 4>, 6> frustum = {};

public:
    void update(Matrix4 const& view, Matrix4 const& projection);

    [[nodiscard]] constexpr bool point_in(Vector3f const& position) const
    {
        for (uint32_t i = 0; i < 6; i++) {
            if (frustum[i][0] * position.x + frustum[i][1] * position.y + frustum[i][2] * position.z + frustum[i][3] <=
                0.0f) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool sphere_in(Vector3f const& position, float radius) const
    {
        for (uint32_t i = 0; i < 6; i++) {
            if (frustum[i][0] * position.x + frustum[i][1] * position.y + frustum[i][2] * position.z + frustum[i][3] <=
                -radius) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool cube_in(Vector3f const& min, Vector3f const& max) const
    {
        for (uint32_t i = 0; i < 6; i++) {
            if (frustum[i][0] * min.x + frustum[i][1] * min.y + frustum[i][2] * min.z + frustum[i][3] <= 0.0f &&
                frustum[i][0] * max.x + frustum[i][1] * min.y + frustum[i][2] * min.z + frustum[i][3] <= 0.0f &&
                frustum[i][0] * min.x + frustum[i][1] * max.y + frustum[i][2] * min.z + frustum[i][3] <= 0.0f &&
                frustum[i][0] * max.x + frustum[i][1] * max.y + frustum[i][2] * min.z + frustum[i][3] <= 0.0f &&
                frustum[i][0] * min.x + frustum[i][1] * min.y + frustum[i][2] * max.z + frustum[i][3] <= 0.0f &&
                frustum[i][0] * max.x + frustum[i][1] * min.y + frustum[i][2] * max.z + frustum[i][3] <= 0.0f &&
                frustum[i][0] * min.x + frustum[i][1] * max.y + frustum[i][2] * max.z + frustum[i][3] <= 0.0f &&
                frustum[i][0] * max.x + frustum[i][1] * max.y + frustum[i][2] * max.z + frustum[i][3] <= 0.0f) {
                return false;
            }
        }

        return true;
    }

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(frustum);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(frustum);
    }

private:
    void normalize(int32_t side);
};
}