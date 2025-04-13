#include "vertex_buffer.hpp"

namespace xen {
VertexBuffer::VertexBuffer(DataType const* data, size_t size, UsageType usage)
{
    load(data, size, usage);
}

size_t VertexBuffer::get_size() const
{
    return get_byte_size() / sizeof(DataType);
}

void VertexBuffer::load(DataType const* data, size_t size, UsageType usage)
{
    Buffer::load((uint8_t const*)data, size * sizeof(DataType), BufferType::ARRAY, usage);
}

void VertexBuffer::buffer_sub_data(DataType const* data, size_t size, size_t offset)
{
    Buffer::buffer_sub_data((uint8_t const*)data, size * sizeof(DataType), offset * sizeof(DataType));
}

void VertexBuffer::buffer_data_with_resize(DataType const* data, size_t size)
{
    Buffer::buffer_data_with_resize((uint8_t const*)data, size * sizeof(DataType));
}

void VertexBuffer::get_buffer_data(DataType* data, size_t size, size_t offset) const
{
    Buffer::get_buffer_data((uint8_t*)data, size * sizeof(DataType), offset * sizeof(DataType));
}
}