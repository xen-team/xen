#pragma once

#include "buffer.hpp"

namespace xen {
class VertexBuffer : public Buffer {
public:
    using DataType = float;

    VertexBuffer(DataType const* data, size_t size, UsageType usage);

    [[nodiscard]] size_t get_size() const;

    void load(DataType const* data, size_t size, UsageType usage);

    void buffer_sub_data(DataType const* data, size_t size, size_t offset = 0);

    void buffer_data_with_resize(DataType const* data, size_t size);

    void get_buffer_data(DataType* data, size_t size, size_t offset = 0) const;
};
}