#include "graphic_objects.hpp"

#include <render/renderer.hpp>

namespace xen {
VertexArray::VertexArray()
{
    Log::debug("[VertexArray] Creating...");
    Renderer::generate_vertex_array(index);
    Log::debug("[VertexArray] Created (ID: " + std::to_string(index) + ")");
}

void VertexArray::bind() const
{
    Renderer::bind_vertex_array(index);
}

void VertexArray::unbind() const
{
    Renderer::unbind_vertex_array();
}

VertexArray::~VertexArray()
{
    if (!index.is_valid()) {
        return;
    }

    Log::debug("[VertexArray] Destroying (ID: " + std::to_string(index) + ")...");
    Renderer::delete_vertex_array(index);
    Log::debug("[VertexArray] Destroyed");
}

VertexBuffer::VertexBuffer()
{
    Log::debug("[VertexBuffer] Creating...");
    Renderer::generate_buffer(index);
    Log::debug("[VertexBuffer] Created (ID: " + std::to_string(index) + ")");
}

void VertexBuffer::bind() const
{
    Renderer::bind_buffer(BufferType::ARRAY_BUFFER, index);
}

void VertexBuffer::unbind() const
{
    Renderer::unbind_buffer(BufferType::ARRAY_BUFFER);
}

VertexBuffer::~VertexBuffer()
{
    if (!index.is_valid()) {
        return;
    }

    Log::debug("[VertexBuffer] Destroying (ID: " + std::to_string(index) + ")...");
    Renderer::delete_buffer(index);
    Log::debug("[VertexBuffer] Destroyed");
}

IndexBuffer::IndexBuffer()
{
    Log::debug("[IndexBuffer] Creating...");
    Renderer::generate_buffer(index);
    Log::debug("[IndexBuffer] Created (ID: " + std::to_string(index) + ")");
}

void IndexBuffer::bind() const
{
    Renderer::bind_buffer(BufferType::ELEMENT_BUFFER, index);
}

void IndexBuffer::unbind() const
{
    Renderer::unbind_buffer(BufferType::ELEMENT_BUFFER);
}

IndexBuffer::~IndexBuffer()
{
    if (!index.is_valid()) {
        return;
    }

    Log::debug("[IndexBuffer] Destroying (ID: " + std::to_string(index) + ")...");
    Renderer::delete_buffer(index);
    Log::debug("[IndexBuffer] Destroyed");
}

}
