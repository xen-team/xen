#pragma once

#include <render/texture/texture.hpp>

namespace xen {
enum class ColorAttachmentFormat : int {
    NormalizedSingleChannel8 = GL_RED,
    Normalized8 = GL_RGBA8,
    Normalized16 = GL_RGBA16,
    FloatingPoint16 = GL_RGBA16F,
    FloatingPoint32 = GL_RGBA32F
};

enum class DepthStencilAttachmentFormat : int {
    NormalizedDepthOnly = GL_DEPTH_COMPONENT,
    NormalizedDepthStencil = GL_DEPTH24_STENCIL8,
    FloatingPointDepthStencil = GL_DEPTH32F_STENCIL8
};

enum StencilValue : uint8_t { ModelStencilValue = 0x01, TerrainStencilValue = 0x02 };

class Framebuffer {
protected:
    uint fbo;
    Vector2ui extent;

    bool multisampled;

    // Render Targets (Attachments)
    // TODO: Maybe do dynamic attachments instead of fixed ones
    Texture color_texture;
    Texture depth_stencil_texture;
    uint depth_stencil_rbo;

public:
    Framebuffer(Vector2ui const& extent, bool multisampled);
    virtual ~Framebuffer();

    // Creation functions
    void create_framebuffer();
    Framebuffer& add_color_texture(ColorAttachmentFormat texture_format);
    Framebuffer& add_depth_stencil_texture(
        DepthStencilAttachmentFormat texture_format, bool bilinear_filtering = false
    ); // bilinearFiltering should be false for GBuffer but shadowmaps can set this to true to get some free bilinear
       // sampling
    Framebuffer& add_depth_stencil_rbo(DepthStencilAttachmentFormat rbo_format);

    void bind();
    void unbind();

    // Assumes framebuffer is bound
    void set_color_attachment(uint target, uint target_type, int mip_to_write_to = 0);
    void set_depth_attachment(DepthStencilAttachmentFormat texture_format, uint target, uint target_type);

    void clear_all();
    void clear_color();
    void clear_depth();
    void clear_stencil();

    // Getters
    [[nodiscard]] uint get_framebuffer() const { return fbo; }

    [[nodiscard]] uint get_width() const { return extent.x; }
    [[nodiscard]] uint get_height() const { return extent.y; }

    [[nodiscard]] bool is_multisampled() const { return multisampled; }

    [[nodiscard]] Texture& get_color_texture() { return color_texture; }

    [[nodiscard]] Texture& get_depth_stencil_texture() { return depth_stencil_texture; }
    [[nodiscard]] uint get_depth_stencil_rbo() const { return depth_stencil_rbo; }
};
}