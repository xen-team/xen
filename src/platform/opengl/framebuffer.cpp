#include "framebuffer.hpp"

namespace xen {
Framebuffer::Framebuffer(Vector2ui const& extent, bool multisampled) :
    fbo(0), extent(extent), multisampled(multisampled), depth_stencil_rbo(0)
{
    GL_CALL(glGenFramebuffers(1, &fbo));
    Log::debug("OpenGL::Framebuffer", " created buffer with id: ", fbo);
}

Framebuffer::~Framebuffer()
{
    GL_CALL(glDeleteRenderbuffers(1, &depth_stencil_rbo));

    GL_CALL(glDeleteFramebuffers(1, &fbo));
    Log::debug("OpenGL::Framebuffer", " deleted buffer with id: ", fbo);
}

void Framebuffer::create_framebuffer()
{
    bind();
    if (!color_texture.is_generated()) {
        // Indicate that there won't be a colour buffer for this FBO
        GL_CALL(glDrawBuffer(GL_NONE));
        GL_CALL(glReadBuffer(GL_NONE));
    }

    // Check if the creation failed
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Log::error("OpenGL::Framebuffer", " could not initialize");
        return;
    }
    unbind();
}

Framebuffer& Framebuffer::add_color_texture(ColorAttachmentFormat texture_format)
{
#ifdef XEN_DEBUG
    if (color_texture.is_generated()) {
        Log::error("OpenGL::Framebuffer", " already has a colour attachment");
        return *this;
    }
#endif

    // "Framebuffer width and height need to be > 0 to generate color texture"
    assert(extent.x > 0 && extent.y > 0);

    bind();

    TextureSettings color_texture_settings;
    color_texture_settings.texture_format = static_cast<GLenum>(texture_format);
    color_texture_settings.texture_wrap_s_mode = GL_CLAMP_TO_EDGE;
    color_texture_settings.texture_wrap_t_mode = GL_CLAMP_TO_EDGE;
    color_texture_settings.texture_minification_filter_mode = GL_LINEAR;
    color_texture_settings.texture_magnification_filter_mode = GL_LINEAR;
    color_texture_settings.texture_anisotropy_level = 1.0f;
    color_texture_settings.has_mips = false;
    color_texture.set_texture_settings(std::move(color_texture_settings));

    // Generate colour texture attachment
    if (multisampled) {
        color_texture.generate_2d_multisample_texture(extent);
        set_color_attachment(color_texture.get_id(), GL_TEXTURE_2D_MULTISAMPLE);
    }
    else {
        color_texture.generate_2d_texture(extent, GL_RGB);
        set_color_attachment(color_texture.get_id(), GL_TEXTURE_2D);
    }

    unbind();
    return *this;
}

Framebuffer& Framebuffer::
    add_depth_stencil_texture(DepthStencilAttachmentFormat texture_format, bool bilinear_filtering /* = false*/)
{
#ifdef XEN_DEBUG
    if (depth_stencil_texture.is_generated()) {
        Log::error("OpenGL::Framebuffer", " already has a depths attachment");
        return *this;
    }
#endif

    // "Framebuffer width and height need to be > 0 to generate depth texture"
    assert(extent.x > 0 && extent.y > 0);

    GLenum attachment_type = GL_DEPTH_STENCIL_ATTACHMENT;
    if (texture_format == DepthStencilAttachmentFormat::NormalizedDepthOnly) {
        attachment_type = GL_DEPTH_ATTACHMENT;
    }

    bind();

    TextureSettings depth_stencil_settings;
    depth_stencil_settings.texture_format = static_cast<GLenum>(texture_format);
    depth_stencil_settings.texture_wrap_s_mode = GL_CLAMP_TO_BORDER;
    depth_stencil_settings.texture_wrap_t_mode = GL_CLAMP_TO_BORDER;
    if (bilinear_filtering) {
        depth_stencil_settings.texture_minification_filter_mode = GL_LINEAR;
        depth_stencil_settings.texture_magnification_filter_mode = GL_LINEAR;
    }
    else {
        depth_stencil_settings.texture_minification_filter_mode = GL_NEAREST;
        depth_stencil_settings.texture_magnification_filter_mode = GL_NEAREST;
    }
    depth_stencil_settings.texture_anisotropy_level = 1.0f;
    depth_stencil_settings.has_border = true;
    depth_stencil_settings.border_color = Vector4f(1.f);
    depth_stencil_settings.has_mips = false;
    depth_stencil_texture.set_texture_settings(std::move(depth_stencil_settings));

    // Generate depth attachment
    if (multisampled) {
        depth_stencil_texture.generate_2d_multisample_texture(extent);
        GL_CALL(glFramebufferTexture2D(
            GL_FRAMEBUFFER, attachment_type, GL_TEXTURE_2D_MULTISAMPLE, depth_stencil_texture.get_id(), 0
        ));
    }
    else {
        depth_stencil_texture.generate_2d_texture(extent, GL_DEPTH_COMPONENT);
        GL_CALL(
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_type, GL_TEXTURE_2D, depth_stencil_texture.get_id(), 0)
        );
    }

    unbind();

    return *this;
}

Framebuffer& Framebuffer::add_depth_stencil_rbo(DepthStencilAttachmentFormat texture_format)
{
#ifdef XEN_DEBUG
    if (depth_stencil_rbo != 0) {
        Log::error("OpenGL::Framebuffer", " already has a depth+stencil RBO attachment");
        return *this;
    }
#endif

    // "Framebuffer width and height need to be > 0 to generate depth/stencil attachment"
    assert(extent.x > 0 && extent.y > 0);

    bind();

    GLenum attachment_type = GL_DEPTH_STENCIL_ATTACHMENT;
    if (texture_format == DepthStencilAttachmentFormat::NormalizedDepthOnly) {
        attachment_type = GL_DEPTH_ATTACHMENT;
    }

    // Generate depth+stencil RBO attachment
    GL_CALL(glGenRenderbuffers(1, &depth_stencil_rbo));
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_rbo));

    if (multisampled) {
        GL_CALL(glRenderbufferStorageMultisample(
            GL_RENDERBUFFER, msaa_sample_amount, static_cast<GLenum>(texture_format), extent.x, extent.y
        ));
    }
    else {
        GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, static_cast<GLenum>(texture_format), extent.x, extent.y));
    }

    // Attach depth+stencil attachment
    GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment_type, GL_RENDERBUFFER, depth_stencil_rbo));

    unbind();
    return *this;
}

void Framebuffer::set_color_attachment(uint target, uint target_type, int mip_to_write_to)
{
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target_type, target, mip_to_write_to));
}

void Framebuffer::set_depth_attachment(DepthStencilAttachmentFormat texture_format, uint target, uint targetType)
{
    GLenum attachment_type = GL_DEPTH_STENCIL_ATTACHMENT;
    if (texture_format == DepthStencilAttachmentFormat::NormalizedDepthOnly) {
        attachment_type = GL_DEPTH_ATTACHMENT;
    }

    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_type, targetType, target, 0));
}

void Framebuffer::bind()
{
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
}

void Framebuffer::unbind()
{
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Framebuffer::clear_all()
{
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void Framebuffer::clear_color()
{
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
}

void Framebuffer::clear_depth()
{
    GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));
}

void Framebuffer::clear_stencil()
{
    GL_CALL(glClear(GL_STENCIL_BUFFER_BIT));
}
}