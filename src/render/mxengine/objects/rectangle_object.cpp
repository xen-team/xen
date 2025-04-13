#include "rectangle_object.hpp"

namespace xen {
void RectangleObject::init(float half_size)
{
    std::array vertices = {
        Vector4f(-half_size, -half_size, 0.5f, 1.0f),
        Vector4f(half_size, -half_size, 0.5f, 1.0f),
        Vector4f(-half_size, half_size, 0.5f, 1.0f),
        Vector4f(half_size, half_size, 0.5f, 1.0f),
    };

    std::array<uint8_t, index_count> indices = {
        0u, 1u, 2u, 2u, 1u, 3u,
    };

    vbo = std::make_unique<VertexBuffer>(
        (float*)vertices.data(), vertices.size() * (sizeof(Vector4f) / sizeof(float)), UsageType::STATIC_DRAW
    );

    ibo = std::make_unique<IndexBuffer>(indices.data(), indices.size(), UsageType::STATIC_DRAW);

    std::array vertex_layout = {VertexAttribute::entry<Vector4f>()};
    vao = std::make_unique<VertexArray>();
    vao->add_vertex_layout(*vbo, vertex_layout, VertexAttributeInputRate::PER_VERTEX);
    vao->link_index_buffer(*ibo);
}

VertexArray const& RectangleObject::get_vao() const
{
    return *vao;
}
}