#include "uniform_buffer.hpp"

#include <render/renderer.hpp>
#include <render/shader/shader_program.hpp>

namespace xen {
namespace {
inline BufferDataUsage recover_data_usage(UniformBufferUsage usage)
{
    switch (usage) {
    case UniformBufferUsage::STATIC:
        return BufferDataUsage::STATIC_DRAW;
    case UniformBufferUsage::DYNAMIC:
        return BufferDataUsage::DYNAMIC_DRAW;
    case UniformBufferUsage::STREAM:
        return BufferDataUsage::STREAM_DRAW;
    }

    throw std::invalid_argument("Error: Invalid uniform buffer usage");
}
}

UniformBuffer::UniformBuffer(uint size, UniformBufferUsage usage) : UniformBuffer()
{
    Log::vdebug("[UniformBuffer] Creating (with size: {})...", size);

    bind();
    Renderer::send_buffer_data(
        BufferType::UNIFORM_BUFFER, size, nullptr, recover_data_usage(usage)
    ); // Allocating memory
    unbind();

    Log::debug("[UniformBuffer] Created (ID: " + std::to_string(index) + ")");
}

void UniformBuffer::bind_uniform_block(ShaderProgram const& program, uint ubo_index, uint shader_binding_index) const
{
    Renderer::bind_uniform_block(program.get_index(), ubo_index, shader_binding_index);
}

void UniformBuffer::bind_uniform_block(
    ShaderProgram const& program, std::string const& ubo_name, uint shader_binding_index
) const
{
    uint const block_index = Renderer::recover_uniform_block_index(program.get_index(), ubo_name.c_str());

    if (block_index == std::numeric_limits<uint>::max()) {
        return; // The uniform buffer is either not declared or unused in the given shader program; not binding anything
    }

    bind_uniform_block(program, block_index, shader_binding_index);
}

void UniformBuffer::bind_base(uint buffer_binding_index) const
{
    Renderer::bind_buffer_base(BufferType::UNIFORM_BUFFER, buffer_binding_index, index);
}

void UniformBuffer::bind_range(uint buffer_binding_index, std::ptrdiff_t offset, std::ptrdiff_t size) const
{
    Renderer::bind_buffer_range(BufferType::UNIFORM_BUFFER, buffer_binding_index, index, offset, size);
}

void UniformBuffer::bind() const
{
    Renderer::bind_buffer(BufferType::UNIFORM_BUFFER, index);
}

void UniformBuffer::unbind() const
{
    Renderer::unbind_buffer(BufferType::UNIFORM_BUFFER);
}

UniformBuffer::~UniformBuffer()
{
    if (!index.is_valid()) {
        return;
    }

    Log::debug("[UniformBuffer] Destroying (ID: " + std::to_string(index) + ")...");
    Renderer::delete_buffer(index);
    Log::debug("[UniformBuffer] Destroyed");
}

UniformBuffer::UniformBuffer()
{
    Renderer::generate_buffer(index);
}

void UniformBuffer::send_data(void const* data, std::ptrdiff_t size, uint offset) const
{
    Renderer::send_buffer_sub_data(BufferType::UNIFORM_BUFFER, offset, size, data);
}
}