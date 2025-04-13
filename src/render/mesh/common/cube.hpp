#pragma once

#include <render/mesh/mesh.hpp>

namespace xen {
class Cube : public Mesh {
public:
    Cube()
    {
        positions = std::vector<Vector3f>{
            Vector3f(-0.5f, -0.5f, -0.5f), Vector3f(0.5f, 0.5f, -0.5f),   Vector3f(0.5f, -0.5f, -0.5f),
            Vector3f(0.5f, 0.5f, -0.5f),   Vector3f(-0.5f, -0.5f, -0.5f), Vector3f(-0.5f, 0.5f, -0.5f),

            Vector3f(-0.5f, -0.5f, 0.5f),  Vector3f(0.5f, -0.5f, 0.5f),   Vector3f(0.5f, 0.5f, 0.5f),
            Vector3f(0.5f, 0.5f, 0.5f),    Vector3f(-0.5f, 0.5f, 0.5f),   Vector3f(-0.5f, -0.5f, 0.5f),

            Vector3f(-0.5f, 0.5f, 0.5f),   Vector3f(-0.5f, 0.5f, -0.5f),  Vector3f(-0.5f, -0.5f, -0.5f),
            Vector3f(-0.5f, -0.5f, -0.5f), Vector3f(-0.5f, -0.5f, 0.5f),  Vector3f(-0.5f, 0.5f, 0.5f),

            Vector3f(0.5f, 0.5f, 0.5f),    Vector3f(0.5f, -0.5f, -0.5f),  Vector3f(0.5f, 0.5f, -0.5f),
            Vector3f(0.5f, -0.5f, -0.5f),  Vector3f(0.5f, 0.5f, 0.5f),    Vector3f(0.5f, -0.5f, 0.5f),

            Vector3f(-0.5f, -0.5f, -0.5f), Vector3f(0.5f, -0.5f, -0.5f),  Vector3f(0.5f, -0.5f, 0.5f),
            Vector3f(0.5f, -0.5f, 0.5f),   Vector3f(-0.5f, -0.5f, 0.5f),  Vector3f(-0.5f, -0.5f, -0.5f),

            Vector3f(-0.5f, 0.5f, -0.5f),  Vector3f(0.5f, 0.5f, 0.5f),    Vector3f(0.5f, 0.5f, -0.5f),
            Vector3f(0.5f, 0.5f, 0.5f),    Vector3f(-0.5f, 0.5f, -0.5f),  Vector3f(-0.5f, 0.5f, 0.5f)
        };

        normals =
            std::vector<Vector3f>{Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f),
                                  Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f),

                                  Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),
                                  Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),

                                  Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f),
                                  Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f),

                                  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),
                                  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),

                                  Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0.0f, -1.0f, 0.0f),
                                  Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0.0f, -1.0f, 0.0f), Vector3f(0.0f, -1.0f, 0.0f),

                                  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),
                                  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f)};

        uvs = std::vector<Vector2f>{Vector2f(0.0f, 0.0f), Vector2f(1.0f, 1.0f), Vector2f(1.0f, 0.0f),
                                    Vector2f(1.0f, 1.0f), Vector2f(0.0f, 0.0f), Vector2f(0.0f, 1.0f),

                                    Vector2f(0.0f, 0.0f), Vector2f(1.0f, 0.0f), Vector2f(1.0f, 1.0f),
                                    Vector2f(1.0f, 1.0f), Vector2f(0.0f, 1.0f), Vector2f(0.0f, 0.0f),

                                    Vector2f(1.0f, 0.0f), Vector2f(1.0f, 1.0f), Vector2f(0.0f, 1.0f),
                                    Vector2f(0.0f, 1.0f), Vector2f(0.0f, 0.0f), Vector2f(1.0f, 0.0f),

                                    Vector2f(1.0f, 0.0f), Vector2f(0.0f, 1.0f), Vector2f(1.0f, 1.0f),
                                    Vector2f(0.0f, 1.0f), Vector2f(1.0f, 0.0f), Vector2f(0.0f, 0.0f),

                                    Vector2f(0.0f, 1.0f), Vector2f(1.0f, 1.0f), Vector2f(1.0f, 0.0f),
                                    Vector2f(1.0f, 0.0f), Vector2f(0.0f, 0.0f), Vector2f(0.0f, 1.0f),

                                    Vector2f(0.0f, 1.0f), Vector2f(1.0f, 0.0f), Vector2f(1.0f, 1.0f),
                                    Vector2f(1.0f, 0.0f), Vector2f(0.0f, 1.0f), Vector2f(0.0f, 0.0f)};

        tangents =
            std::vector<Vector3f>{Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f),
                                  Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f),

                                  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),
                                  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),

                                  Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),
                                  Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),

                                  Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f),
                                  Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f),

                                  Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f),
                                  Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(-1.0f, 0.0f, 0.0f),

                                  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),
                                  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f),  Vector3f(1.0f, 0.0f, 0.0f)};

        bitangents = std::vector<Vector3f>{
            Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),
            Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),

            Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),
            Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),

            Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),
            Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),

            Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),
            Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),  Vector3f(0.0f, 1.0f, 0.0f),

            Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),
            Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),  Vector3f(0.0f, 0.0f, 1.0f),

            Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f),
            Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, -1.0f)
        };

        load_data();
        generate_gpu_data();
    }
};
}