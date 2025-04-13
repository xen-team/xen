#pragma once

#include "platform/graphic_api.hpp"

namespace xen {
class RenderHelperObject {
protected:
    size_t vertex_offset, vertex_count;
    size_t index_offset, index_count;
    std::unique_ptr<VertexArray> vao;
    std::unique_ptr<VertexBuffer> vbo;
    std::unique_ptr<IndexBuffer> ibo;

public:
    RenderHelperObject() = default;
    RenderHelperObject(
        size_t vertex_offset, size_t vertex_count, size_t index_offset, size_t index_count,
        std::unique_ptr<VertexArray>&& vao = std::make_unique<VertexArray>(),
        std::unique_ptr<VertexBuffer>&& vbo = std::make_unique<VertexBuffer>(nullptr, 0, UsageType::DYNAMIC_COPY),
        std::unique_ptr<IndexBuffer>&& ibo = std::make_unique<IndexBuffer>(nullptr, 0, UsageType::DYNAMIC_COPY)
    ) :
        vertex_offset(vertex_offset), vertex_count(vertex_count), index_offset(index_offset), index_count(index_count),
        vao(std::move(vao)), vbo(std::move(vbo)), ibo(std::move(ibo))
    {
    }

    [[nodiscard]] VertexArray& get_vao() const { return *vao; }
    [[nodiscard]] VertexBuffer& get_vbo() const { return *vbo; }
    [[nodiscard]] IndexBuffer& get_ibo() const { return *ibo; }
    [[nodiscard]] size_t get_index_count() const { return index_count; }
    [[nodiscard]] size_t get_vertex_count() const { return vertex_count; }
    [[nodiscard]] size_t get_index_offset() const { return index_offset; }
    [[nodiscard]] size_t get_vertex_offset() const { return vertex_offset; }
};
}