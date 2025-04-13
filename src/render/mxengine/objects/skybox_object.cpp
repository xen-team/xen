#include "skybox_object.hpp"

namespace xen {
void SkyboxObject::init()
{
    constexpr float size = 1.0f;
    constexpr std::array vertices = {
        Vector3f(-size, -size, -size), Vector3f(-size, -size, size), Vector3f(-size, size, -size),
        Vector3f(-size, size, size),   Vector3f(size, -size, -size), Vector3f(size, -size, size),
        Vector3f(size, size, -size),   Vector3f(size, size, size),
    };
    constexpr std::array<uint8_t, index_count> indices = {
        1u, 7u, 5u, 7u, 1u, 3u, 4u, 2u, 0u, 2u, 4u, 6u, 4u, 7u, 6u, 7u, 4u, 5u,
        2u, 1u, 0u, 1u, 2u, 3u, 0u, 5u, 4u, 5u, 0u, 1u, 6u, 3u, 2u, 3u, 6u, 7u,
    };

    // vbo = Factory<VertexBuffer>::Create(
    //     (float*)vertices.data(), vertices.size() * sizeof(Vector3f), UsageType::STATIC_DRAW
    // );

    vbo = std::make_unique<VertexBuffer>(
        (float*)vertices.data(), vertices.size() * sizeof(Vector3f), UsageType::STATIC_DRAW
    );

    ibo = std::make_unique<IndexBuffer>(indices.data(), indices.size(), UsageType::STATIC_DRAW);

    std::array vertex_layout = {VertexAttribute::entry<Vector3f>()};
    vao = std::make_unique<VertexArray>();
    vao->add_vertex_layout(*vbo, vertex_layout, VertexAttributeInputRate::PER_VERTEX);
    vao->link_index_buffer(*ibo);
}

VertexArray const& SkyboxObject::get_vao() const
{
    return *(this->vao);
}
}