#pragma once

#include "buffer_base.hpp"

namespace xen {
using IndexBufferHandle = BufferBaseHandle;

class IndexBuffer : public BufferBase {
public:
    using IndexType = uint8_t;

    IndexBuffer(IndexType const* data, size_t size, UsageType usage);

    [[nodiscard]] size_t get_size() const;

    void load(IndexType const* data, size_t size, UsageType usage);

    void buffer_sub_data(IndexType const* data, size_t size, size_t offset = 0);

    void buffer_data_with_resize(IndexType const* data, size_t size);

    void get_buffer_data(IndexType* data, size_t size, size_t offset = 0) const;
};
}