#pragma once

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

class Buffer {
private:
    uint id;
    size_t byte_size;
    BufferType type = BufferType::UNKNOWN;
    UsageType usage = UsageType::STATIC_DRAW;

public:
    Buffer();
    Buffer(
        uint32_t byte_size, BufferType type = BufferType::ARRAY,
        UsageType usage = UsageType::STATIC_DRAW
    ); // Can be used to allocate a big buffer then you can use SetData to vary its size
    Buffer(
        uint8_t* data, size_t byte_size, BufferType type = BufferType::ARRAY, UsageType usage = UsageType::STATIC_DRAW
    );
    ~Buffer();

    // Should use one, or the other depending on how this buffer is being used (statically vs dynamically which is
    // 'Load()' vs 'SetData()' respectively)
    void set_data(void const* data, uint32_t byte_size);

    void load(
        uint8_t const* data, size_t byte_size, BufferType type = BufferType::ARRAY,
        UsageType usage = UsageType::STATIC_DRAW
    );

    void buffer_sub_data(uint8_t const* data, size_t byte_size, size_t offset = 0);

    void buffer_data_with_resize(uint8_t const* data, size_t byte_size);

    void get_buffer_data(uint8_t* data, size_t byte_size, size_t offset = 0) const;

    void bind() const;

    void unbind() const;

    void bind_base(size_t index) const;

    [[nodiscard]] BufferType get_buffer_type() const { return type; }

    void set_usage_type(UsageType usage) { this->usage = usage; }
    [[nodiscard]] UsageType get_usage_type() const { return usage; }

    [[nodiscard]] size_t get_byte_size() const { return byte_size; }
};
}