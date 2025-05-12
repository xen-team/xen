#pragma once

#include <data/owner_value.hpp>

namespace xen {
class RenderShaderProgram;
class Texture2D;
using Texture2DPtr = std::shared_ptr<Texture2D>;
class VertexShader;

/// Framebuffer class, handling buffers used for deferred rendering.
class Framebuffer {
    friend class RenderPass;

public:
    Framebuffer();
    Framebuffer(Framebuffer const&) = delete;
    Framebuffer(Framebuffer&&) noexcept = default;

    Framebuffer& operator=(Framebuffer const&) = delete;
    Framebuffer& operator=(Framebuffer&&) noexcept = default;

    ~Framebuffer();

    [[nodiscard]] uint get_index() const { return index; }

    [[nodiscard]] bool empty() const { return (!has_depth_buffer() && color_buffers.empty()); }

    [[nodiscard]] bool has_depth_buffer() const { return (depth_buffer != nullptr); }

    [[nodiscard]] Texture2D const& get_depth_buffer() const
    {
        Log::rt_assert("Error: Framebuffer doesn't contain a depth buffer." && has_depth_buffer());
        return *depth_buffer;
    }

    [[nodiscard]] size_t get_color_buffer_count() const { return color_buffers.size(); }

    [[nodiscard]] Texture2D const& get_color_buffer(size_t buffer_index) const
    {
        return *color_buffers[buffer_index].first;
    }

    /// Gives a basic vertex shader, to display the framebuffer.
    /// \return Basic display vertex shader.
    static VertexShader recover_vertex_shader();

    /// Sets the write depth buffer texture.
    /// \param texture Depth buffer texture to be set; must have a depth colorspace.
    void set_depth_buffer(Texture2DPtr texture);

    /// Adds a write color buffer texture.
    /// \param texture Color buffer texture to be added; must have a non-depth colorspace.
    /// \param index Buffer's index (location of the shader's output value).
    void add_color_buffer(Texture2DPtr texture, uint index);

    /// Removes a write buffer texture.
    /// \param texture Buffer texture to be removed.
    void remove_texture_buffer(Texture2DPtr const& texture);

    /// Removes the depth buffer.
    void clear_depth_buffer() { depth_buffer.reset(); }

    /// Removes all color buffers.
    void clear_color_buffers() { color_buffers.clear(); }

    /// Removes both depth & color buffers.
    void clear_texture_buffers();

    /// Resizes the buffer textures.
    /// \param width Width to be resized to.
    /// \param height Height to be resized to.
    void resize_buffers(Vector2ui const& size);

    /// Maps the buffers textures onto the graphics card.
    void map_buffers() const;

    /// Binds the framebuffer and clears the color & depth buffers.
    void bind() const;

    /// Unbinds the framebuffer.
    void unbind() const;

    /// Displays the framebuffer.
    void display() const;

private:
    OwnerValue<uint> index{};
    Texture2DPtr depth_buffer{};
    std::vector<std::pair<Texture2DPtr, uint>> color_buffers{};
};
}