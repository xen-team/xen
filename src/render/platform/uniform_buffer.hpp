#pragma once

#include <data/owner_value.hpp>

namespace xen {
class ShaderProgram;

enum class UniformBufferUsage {
    STATIC,  ///< Data is assumed to never change.
    DYNAMIC, ///< Data is assumed to be changed often.
    STREAM   ///< Data is assumed to be given each frame.
};

class UniformBuffer {
public:
    explicit UniformBuffer(uint size, UniformBufferUsage usage = UniformBufferUsage::DYNAMIC);

    UniformBuffer(UniformBuffer const&) = delete;
    UniformBuffer(UniformBuffer&&) noexcept = default;

    UniformBuffer& operator=(UniformBuffer const&) = delete;
    UniformBuffer& operator=(UniformBuffer&&) noexcept = default;

    ~UniformBuffer();

    [[nodiscard]] uint get_index() const { return index; }

    void bind_uniform_block(ShaderProgram const& program, uint ubo_index, uint shader_binding_index) const;

    void bind_uniform_block(ShaderProgram const& program, std::string const& ubo_name, uint shader_binding_index) const;

    void bind_base(uint buffer_binding_index) const;

    void bind_range(uint buffer_binding_index, std::ptrdiff_t offset, std::ptrdiff_t size) const;

    void bind() const;

    void unbind() const;

    template <typename T>
    void send_data(T const& data, uint offset) const
    {
        send_data(&data, sizeof(T), offset);
    }

    template <typename T>
    void send_data(Vector2<T> const& data, uint offset) const
    {
        send_data(&data[0], sizeof(data), offset);
    }
    template <typename T>
    void send_data(Vector3<T> const& data, uint offset) const
    {
        send_data(&data[0], sizeof(data), offset);
    }
    template <typename T>
    void send_data(Vector4<T> const& data, uint offset) const
    {
        send_data(&data[0], sizeof(data), offset);
    }

    void send_data(Matrix2 const& data, uint offset) const { send_data(&data[0], sizeof(data), offset); }
    void send_data(Matrix3 const& data, uint offset) const { send_data(&data[0], sizeof(data), offset); }
    void send_data(Matrix4 const& data, uint offset) const { send_data(&data[0], sizeof(data), offset); }

private:
    OwnerValue<uint> index;

private:
    UniformBuffer();

    void send_data(void const* data, std::ptrdiff_t size, uint offset) const;
};
}