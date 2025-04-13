#pragma once

#include "buffer_base.hpp"

namespace xen {
class ShaderStorageBuffer : public BufferBase {
public:
    template <typename T>
    ShaderStorageBuffer(T const* data, size_t size, UsageType usage)
    {
        load<T>(data, size, usage);
    }

    template <typename T>
    [[nodiscard]] size_t get_size() const
    {
        return get_byte_size() / sizeof(T);
    }

    template <typename T>
    void load(T const* data, size_t size, UsageType usage)
    {
        BufferBase::load(BufferType::SHADER_STORAGE, (uint8_t const*)data, size * sizeof(T), usage);
    }

    template <typename T>
    void buffer_sub_data(T const* data, size_t size, size_t offset = 0)
    {
        BufferBase::buffer_sub_data((uint8_t const*)data, size * sizeof(T), offset * sizeof(T));
    }

    template <typename T>
    void buffer_data_with_resize(T const* data, size_t size)
    {
        BufferBase::buffer_data_with_resize((uint8_t const*)data, size * sizeof(T));
    }

    template <typename T>
    void get_buffer_data(T* data, size_t size, size_t offset = 0)
    {
        BufferBase::get_buffer_data((uint8_t*)data, size * sizeof(T), offset * sizeof(T));
    }

    void BindBase(size_t index) const { BufferBase::bind_base(index); }
};
}