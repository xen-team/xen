#pragma once

#include <cstdint>
#include <cstddef>

namespace xen {
enum class UsageType : uint8_t {
    STREAM_DRAW,
    STREAM_READ,
    STREAM_COPY,
    STATIC_DRAW,
    STATIC_READ,
    STATIC_COPY,
    DYNAMIC_DRAW,
    DYNAMIC_READ,
    DYNAMIC_COPY,
};

enum class BufferType : uint8_t {
    UNKNOWN,
    ARRAY,
    ELEMENT_ARRAY,
    SHADER_STORAGE,
};

using BufferBaseHandle = uint32_t;

class BufferBase {
private:
    BufferBaseHandle id = 0;
    size_t byte_size = 0;
    BufferType type = BufferType::UNKNOWN;
    UsageType usage = UsageType::STATIC_DRAW;

public:
    BufferBase();
    ~BufferBase();
    BufferBase(BufferBase const&) = delete;
    BufferBase(BufferBase&&) noexcept;
    BufferBase& operator=(BufferBase const&) = delete;
    BufferBase& operator=(BufferBase&&) noexcept;

    void bind() const;

    void unbind() const;

    void bind_base(size_t index) const;

    [[nodiscard]] BufferBaseHandle get_native_handle() const { return id; }

    [[nodiscard]] BufferType get_buffer_type() const { return type; }

    [[nodiscard]] UsageType get_usage_type() const { return usage; }

    [[nodiscard]] size_t get_byte_size() const { return byte_size; }

    void set_usage_type(UsageType usage) { this->usage = usage; }

    void load_from(BufferBase& other);

private:
    void free();

protected:
    void load(BufferType type, uint8_t const* byte_data, size_t byte_size, UsageType usage);

    void buffer_sub_data(uint8_t const* byte_data, size_t byte_size, size_t offset = 0);

    void buffer_data_with_resize(uint8_t const* byte_data, size_t byte_size);

    void get_buffer_data(uint8_t* byte_data, size_t byte_size, size_t offset = 0) const;
};
}