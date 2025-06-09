#pragma once

#include <data/owner_value.hpp>

namespace xen {
class VertexArray {
public:
    VertexArray();
    VertexArray(VertexArray const&) = delete;
    VertexArray(VertexArray&&) noexcept = default;

    VertexArray& operator=(VertexArray const&) = delete;
    VertexArray& operator=(VertexArray&&) noexcept = default;

    ~VertexArray();

    [[nodiscard]] uint32_t get_index() const { return index; }

    void bind() const;

    void unbind() const;

private:
    OwnerValue<uint32_t> index{0};
};

class VertexBuffer {
public:
    uint32_t vertex_count{0};

public:
    VertexBuffer();
    VertexBuffer(VertexBuffer const&) = delete;
    VertexBuffer(VertexBuffer&&) noexcept = default;

    VertexBuffer& operator=(VertexBuffer const&) = delete;
    VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

    ~VertexBuffer();

    [[nodiscard]] uint32_t get_index() const { return index; }

    void bind() const;
    void unbind() const;

private:
    OwnerValue<uint32_t> index{0};
};

class IndexBuffer {
public:
    uint32_t line_index_count{0};
    uint32_t triangle_index_count{0};

public:
    IndexBuffer();
    IndexBuffer(IndexBuffer const&) = delete;
    IndexBuffer(IndexBuffer&&) noexcept = default;

    IndexBuffer& operator=(IndexBuffer const&) = delete;
    IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

    ~IndexBuffer();

    [[nodiscard]] uint32_t get_index() const { return index; }

    void bind() const;
    void unbind() const;

private:
    OwnerValue<uint32_t> index{0};
};
}