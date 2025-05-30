#pragma once

#include "utils/shape.hpp"
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

    [[nodiscard]] constexpr bool aabb_in(AABB const& box) const
    {
        Vector3f const& min = box.get_min_position();
        Vector3f const& max = box.get_max_position();

        for (uint32_t i = 0; i < 6; ++i) {
            float const nx = frustum[i][0];
            float const ny = frustum[i][1];
            float const nz = frustum[i][2];
            float const d = frustum[i][3];

            float p_vertex_x = (nx >= 0) ? max.x : min.x;
            float p_vertex_y = (ny >= 0) ? max.y : min.y;
            float p_vertex_z = (nz >= 0) ? max.z : min.z;

            if ((nx * p_vertex_x + ny * p_vertex_y + nz * p_vertex_z + d) <= 0.0f) {
                return false;
            }
        }
        return true;
    }

private:
    void normalize(int32_t side);
};
}