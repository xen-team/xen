#pragma once

#include <data/graph.hpp>
#include <render/platform/framebuffer.hpp>
#if !defined(USE_OPENGL_ES)
#include <render/render_timer.hpp>
#endif
#include <render/shader/shader_program.hpp>
#include <render/texture.hpp>

namespace xen {
class RenderPass final : public GraphNode<RenderPass> {
    friend class RenderGraph;

public:
    RenderPass() = default;
    RenderPass(VertexShader&& vert_shader, FragmentShader&& frag_shader, std::string pass_name = {}) :
        name{std::move(pass_name)}, program(std::move(vert_shader), std::move(frag_shader))
    {
    }
    explicit RenderPass(FragmentShader&& frag_shader, std::string pass_name = {}) :
        RenderPass(Framebuffer::recover_vertex_shader(), std::move(frag_shader), std::move(pass_name))
    {
    }
    RenderPass(RenderPass const&) = delete;
    RenderPass(RenderPass&&) noexcept = default;

    RenderPass& operator=(RenderPass const&) = delete;
    RenderPass& operator=(RenderPass&&) noexcept = default;

    ~RenderPass() override = default;

    [[nodiscard]] bool is_enabled() const { return enabled; }

    [[nodiscard]] std::string const& get_name() const { return name; }

    [[nodiscard]] RenderShaderProgram const& get_program() const { return program; }

    [[nodiscard]] RenderShaderProgram& get_program() { return program; }

    [[nodiscard]] size_t get_read_texture_count() const { return program.get_texture_count(); }

    [[nodiscard]] Texture const& get_read_texture(size_t texture_index) const
    {
        return program.get_texture(texture_index);
    }

    [[nodiscard]] bool has_read_texture(std::string const& uniform_name) const
    {
        return program.has_texture(uniform_name);
    }

    [[nodiscard]] Texture const& get_read_texture(std::string const& uniform_name) const
    {
        return program.get_texture(uniform_name);
    }

    [[nodiscard]] Framebuffer const& get_framebuffer() const { return write_framebuffer; }

    /// Recovers the elapsed time (in milliseconds) of the pass' execution.
    /// \note This action is not available with OpenGL ES and will always return 0.
    /// \return Time taken to execute the pass.
    [[nodiscard]] float recover_elapsed_time() const
    {
#if !defined(USE_OPENGL_ES)
        return timer.recover_time();
#else
        return 0.f;
#endif
    }

    void set_name(std::string name) { this->name = std::move(name); }

    void set_program(RenderShaderProgram&& program) { this->program = std::move(program); }

    /// Changes the render pass' enabled state.
    /// \param enabled True if the render pass should be enabled, false if it should be disabled.
    void enable(bool enabled = true) { this->enabled = enabled; }

    /// Disables the render pass.
    void disable() { enable(false); }

    /// Checks that the current render pass is valid, that is, if none of its buffer has been defined as both read &
    /// write.
    /// \return True if the render pass is valid, false otherwise.
    /// \see RenderGraph::is_valid()
    bool is_valid() const;

    void add_read_texture(TexturePtr texture, std::string const& uniform_name);

    void remove_read_texture(Texture const& texture) { program.remove_texture(texture); }

    void clear_read_textures() { program.clear_textures(); }

    /// Sets the write depth buffer texture.
    /// \param texture Depth buffer texture to be set; must have a depth colorspace.
    void set_write_depth_texture(Texture2DPtr texture) { write_framebuffer.set_depth_buffer(std::move(texture)); }

    /// Adds a write color buffer texture.
    /// \param texture Color buffer texture to be added; must have a non-depth colorspace.
    /// \param index Buffer's index (location of the shader's output value).
    void add_write_color_texture(Texture2DPtr texture, uint index)
    {
        write_framebuffer.add_color_buffer(std::move(texture), index);
    }

    void remove_write_texture(Texture2DPtr const& texture) { write_framebuffer.remove_texture_buffer(texture); }

    void clear_write_textures() { write_framebuffer.clear_texture_buffers(); }

    /// Resizes the render pass' write buffer textures.
    /// \param width New buffers width.
    /// \param height New buffers height.
    void resize_write_buffers(Vector2ui const& size) { write_framebuffer.resize_buffers(size); }

    /// Executes the render pass.
    void execute() const;

private:
    bool enabled = true;
    std::string name{};
    RenderShaderProgram program{};
    Framebuffer write_framebuffer{};

#if !defined(USE_OPENGL_ES)
    RenderTimer timer{};
#endif
};
}