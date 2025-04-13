#include "index_buffer.hpp"

namespace xen {
IndexBuffer::IndexBuffer(IndexType const* data, size_t size, UsageType usage)
{
    load(data, size, usage);
}

size_t IndexBuffer::get_size() const
{
    return get_byte_size() / sizeof(IndexType);
}

void IndexBuffer::load(IndexType const* data, size_t size, UsageType usage)
{
    BufferBase::load(BufferType::ARRAY, data, size * sizeof(IndexType), usage);
}

void IndexBuffer::buffer_sub_data(IndexType const* data, size_t size, size_t offset)
{
    BufferBase::buffer_sub_data(data, size * sizeof(IndexType), offset * sizeof(IndexType));
}

void IndexBuffer::buffer_data_with_resize(IndexType const* data, size_t size)
{
    BufferBase::buffer_data_with_resize(data, size * sizeof(IndexType));
}

void IndexBuffer::get_buffer_data(IndexType* data, size_t size, size_t offset) const
{
    BufferBase::get_buffer_data(data, size * sizeof(IndexType), offset * sizeof(IndexType));
}
}