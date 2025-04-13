#include "index_buffer.hpp"

namespace xen {
IndexBuffer::IndexBuffer(DataType const* data, size_t size, UsageType usage)
{
    load(data, size, usage);
}

size_t IndexBuffer::get_size() const
{
    return get_byte_size() / sizeof(DataType);
}

void IndexBuffer::load(DataType const* data, size_t size, UsageType usage)
{
    Buffer::load(data, size * sizeof(DataType), BufferType::ARRAY, usage);
}

void IndexBuffer::buffer_sub_data(DataType const* data, size_t size, size_t offset)
{
    Buffer::buffer_sub_data(data, size * sizeof(DataType), offset * sizeof(DataType));
}

void IndexBuffer::buffer_data_with_resize(DataType const* data, size_t size)
{
    Buffer::buffer_data_with_resize(data, size * sizeof(DataType));
}

void IndexBuffer::get_buffer_data(DataType* data, size_t size, size_t offset) const
{
    Buffer::get_buffer_data(data, size * sizeof(DataType), offset * sizeof(DataType));
}
}