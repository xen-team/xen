#pragma once

#include <render/mesh/mesh.hpp>

namespace xen {
class Quad : public Mesh {
public:
    Quad(bool ndc_quad = true)
    {
        if (ndc_quad) {
            AddVertex(Vector3f(-1, 1, 0), Vector2f(0, 1));
            AddVertex(Vector3f(1, 1, 0), Vector2f(1, 1));
            AddVertex(Vector3f(-1, -1, 0), Vector2f(0, 0));
            AddVertex(Vector3f(1, -1, 0), Vector2f(1, 0));
        }
        else {
            AddVertex(Vector3f(-0.5f, 0.5f, 0), Vector2f(0, 0));
            AddVertex(Vector3f(0.5f, 0.5f, 0), Vector2f(1, 0));
            AddVertex(Vector3f(-0.5f, -0.5f, 0), Vector2f(0, 1));
            AddVertex(Vector3f(0.5f, -0.5f, 0), Vector2f(1, 1));
        }

        indices = {1, 0, 2, 3, 1, 2};

        load_data();
        generate_gpu_data();
    }

private:
    void AddVertex(Vector3f const& position, Vector2f const& uv)
    {
        positions.emplace_back(position);
        uvs.emplace_back(uv);
        normals.emplace_back(Vector3f(0, 0, 1));
        tangents.emplace_back(Vector3f(1, 0, 0));
        bitangents.emplace_back(Vector3f(0, 1, 0));
    }
};
}