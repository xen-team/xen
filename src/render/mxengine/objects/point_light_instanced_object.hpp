#pragma once

#include "render_helper_object.hpp"

namespace xen {
struct PointLightBaseData {
    Matrix4 transform;
    Vector3f position;
    Vector3f color;
    float radius;
    float ambient_intensity;

    constexpr static size_t size = 16 + 3 + 1 + 3 + 1;
};

class PointLightInstancedObject : public RenderHelperObject {
private:
    std::unique_ptr<VertexBuffer> instanced_vbo;

public:
    std::vector<PointLightBaseData> instances;

public:
    PointLightInstancedObject() = default;

    PointLightInstancedObject(size_t vertexOffset, size_t vertexCount, size_t indexOffset, size_t indexCount) :
        RenderHelperObject(vertexOffset, vertexCount, indexOffset, indexCount)
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
            VertexAttribute::entry<Vector4f>(), // position + radius
            VertexAttribute::entry<Vector4f>(), // color + ambient
        };

        vao->add_vertex_layout(get_vbo(), vertex_layout, VertexAttributeInputRate::PER_VERTEX);
        vao->add_vertex_layout(*instanced_vbo, instance_layout, VertexAttributeInputRate::PER_INSTANCE);
        vao->link_index_buffer(get_ibo());
    }

    void SubmitToVBO()
    {
        instanced_vbo->buffer_data_with_resize((float*)instances.data(), instances.size() * PointLightBaseData::size);
    }
};
}