#pragma once

namespace xen {
class VertexBuffer;
class IndexBuffer;
struct VertexAttribute;

enum class VertexAttributeInputRate {
    PER_VERTEX,
    PER_INSTANCE,
};

using VertexArrayHandle = uint32_t;

class VertexArray {
private:
    VertexArrayHandle id = 0;
    int attribute_index = 0;

public:
    VertexArray();
    ~VertexArray();
    VertexArray(VertexArray const&) = delete;
    VertexArray(VertexArray&& array) noexcept;
    VertexArray& operator=(VertexArray const&) = delete;
    VertexArray& operator=(VertexArray&& array) noexcept;

    [[nodiscard]] VertexArrayHandle get_native_handle() const { return id; }

    [[nodiscard]] int get_attribute_count() const { return attribute_index; }

    void bind() const;

    void unbind() const;

    void add_vertex_layout(
        VertexBuffer const& buffer, std::span<VertexAttribute> layout, VertexAttributeInputRate input_rate
    );

    void remove_vertex_layout(std::span<VertexAttribute> layout);

    void link_index_buffer(IndexBuffer const& buffer);

private:
    void free();
};
}