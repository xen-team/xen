#pragma once

#include "render_helper_object.hpp"

namespace xen {
struct SpotLightBaseData {
    Matrix4 transform;
    Vector3f position;
    Vector3f direction;
    Vector3f color;
    float inner_angle;
    float outer_angle;
    float ambient_intensity;

    constexpr static size_t size = 16 + 3 + 1 + 3 + 1 + 3 + 1;
};

class SpotLightInstancedObject : public RenderHelperObject {
private:
    std::unique_ptr<VertexBuffer> instanced_vbo;

public:
    std::vector<SpotLightBaseData> instances;

public:
    SpotLightInstancedObject() = default;

    SpotLightInstancedObject(size_t vertex_offset, size_t vertex_count, size_t index_offset, size_t index_count) :
        RenderHelperObject(vertex_offset, vertex_count, index_offset, index_count)
    {
        instanced_vbo = std::make_unique<VertexBuffer>(nullptr, 0, UsageType::STATIC_DRAW);

        std::array vertex_layout = {
            VertexAttribute::entry<Vector3f>(), // position
            VertexAttribute::entry<Vector2f>(), // texture uv
            VertexAttribute::entry<Vector3f>(), // normal
            VertexAttribute::entry<Vector3f>(), // tangent
            VertexAttribute::entry<Vector3f>(), // bitangent
        };
        std::array instance_layout = {
            VertexAttribute::entry<Matrix4>(),  // transform
            VertexAttribute::entry<Vector4f>(), // position + inner angle
            VertexAttribute::entry<Vector4f>(), // direction + outer angle
            VertexAttribute::entry<Vector4f>(), // color + ambient
        };

        vao->add_vertex_layout(get_vbo(), vertex_layout, VertexAttributeInputRate::PER_VERTEX);
        vao->add_vertex_layout(*instanced_vbo, instance_layout, VertexAttributeInputRate::PER_INSTANCE);
        vao->link_index_buffer(get_ibo());
    }

    void submit_to_vbo()
    {
        instanced_vbo->buffer_data_with_resize((float*)instances.data(), instances.size() * SpotLightBaseData::size);
    }
};
}