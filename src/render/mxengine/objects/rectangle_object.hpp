#pragma once

#include "platform/graphic_api.hpp"

namespace xen {
class RectangleObject {
private:
    std::unique_ptr<VertexBuffer> vbo;
    std::unique_ptr<VertexArray> vao;
    std::unique_ptr<IndexBuffer> ibo;

public:
    static constexpr size_t index_count = 2 * 3; // 2 triangles, each with 3 vertecies

    void init(float half_size);
    [[nodiscard]] VertexArray const& get_vao() const;
};
}