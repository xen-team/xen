#pragma once

#include "buffer_base.hpp"

namespace xen {
using VertexBufferHandle = BufferBaseHandle;

class VertexBuffer : public BufferBase {
public:
    using VertexScalar = float;

    VertexBuffer(VertexScalar const* data, size_t size, UsageType usage);

    [[nodiscard]] size_t get_size() const;

    void load(VertexScalar const* data, size_t size, UsageType usage);

    void buffer_sub_data(VertexScalar const* data, size_t size, size_t offset = 0);

    void buffer_data_with_resize(VertexScalar const* data, size_t size);

    void get_buffer_data(VertexScalar* data, size_t size, size_t offset = 0) const;
};
}