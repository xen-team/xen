#include "vertex_buffer.hpp"

namespace xen {
VertexBuffer::VertexBuffer(VertexScalar const* data, size_t size, UsageType usage)
{
    load(data, size, usage);
}

size_t VertexBuffer::get_size() const
{
    return get_byte_size() / sizeof(VertexScalar);
}

void VertexBuffer::load(VertexScalar const* data, size_t size, UsageType usage)
{
    BufferBase::load(BufferType::ARRAY, (uint8_t const*)data, size * sizeof(VertexScalar), usage);
}

void VertexBuffer::buffer_sub_data(VertexScalar const* data, size_t size, size_t offset)
{
    BufferBase::buffer_sub_data((uint8_t const*)data, size * sizeof(VertexScalar), offset * sizeof(VertexScalar));
}

void VertexBuffer::buffer_data_with_resize(VertexScalar const* data, size_t size)
{
    BufferBase::buffer_data_with_resize((uint8_t const*)data, size * sizeof(VertexScalar));
}

void VertexBuffer::get_buffer_data(VertexScalar* data, size_t size, size_t offset) const
{
    BufferBase::get_buffer_data((uint8_t*)data, size * sizeof(VertexScalar), offset * sizeof(VertexScalar));
}
}