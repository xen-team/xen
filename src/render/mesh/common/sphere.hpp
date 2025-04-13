#pragma once

#include <render/mesh/mesh.hpp>

namespace xen {
class Sphere : public Mesh {
public:
    Sphere(Vector2ui const& segments)
    {
        for (uint32_t y = 0; y <= segments.y; ++y) {
            for (uint32_t x = 0; x <= segments.x; ++x) {
                float x_segment = static_cast<float>(x) / static_cast<float>(segments.x);
                float y_segment = static_cast<float>(y) / static_cast<float>(segments.y);
                float x_pos = std::cos(x_segment * Math::PI<float> * 2.0f) * std::sin(y_segment * Math::PI<float>);
                float y_pos = std::cos(y_segment * Math::PI<float>);
                float z_pos = std::sin(x_segment * Math::PI<float> * 2.0f) * std::sin(y_segment * Math::PI<float>);

                positions = {Vector3f(x_pos, y_pos, z_pos)};
                uvs = {Vector2f(x_segment, y_segment)};
                normals = {Vector3f(x_pos, y_pos, z_pos)};
            }
        }

        for (uint32_t y = 0; y < segments.y; ++y) {
            for (uint32_t x = 0; x < segments.x; ++x) {
                indices.emplace_back((y + 1) * (segments.x + 1) + x);
                indices.emplace_back(y * (segments.x + 1) + x);
                indices.emplace_back(y * (segments.x + 1) + x + 1);

                indices.emplace_back((y + 1) * (segments.x + 1) + x);
                indices.emplace_back(y * (segments.x + 1) + x + 1);
                indices.emplace_back((y + 1) * (segments.x + 1) + x + 1);
            }
        }

        load_data();
        generate_gpu_data();
    }
};
}