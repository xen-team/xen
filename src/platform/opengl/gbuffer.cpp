#include "gbuffer.hpp"

namespace xen {
GBuffer::GBuffer(Vector2ui const& extent) : Framebuffer(extent, false)
{
    init();
}

GBuffer::~GBuffer()
{
    Log::debug("OpenGL::GBuffer", " deleted buffer");
}

void GBuffer::init()
{
    add_depth_stencil_texture(DepthStencilAttachmentFormat::NormalizedDepthStencil);

    bind();

    auto create_render_target = [&](int index, GLenum format, GLenum attachment) {
        TextureSettings settings;
        settings.texture_format = format;
        settings.texture_wrap_s_mode = GL_CLAMP_TO_EDGE;
        settings.texture_wrap_t_mode = GL_CLAMP_TO_EDGE;
        settings.texture_minification_filter_mode = GL_NEAREST;
        settings.texture_magnification_filter_mode = GL_NEAREST;
        settings.texture_anisotropy_level = 1.0f;
        settings.has_mips = false;
        gbuffer_render_targets[index].set_texture_settings(std::move(settings));
        gbuffer_render_targets[index].generate_2d_texture(extent, GL_RGB);
        GL_CALL(
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, gbuffer_render_targets[index].get_id(), 0)
        );
    };

    create_render_target(0, GL_RGBA8, GL_COLOR_ATTACHMENT0);

    create_render_target(1, GL_RGB32F, GL_COLOR_ATTACHMENT1);

    create_render_target(2, GL_RGBA8, GL_COLOR_ATTACHMENT2);

    // Finally tell OpenGL that we will be rendering to all of the attachments
    constexpr std::array<uint, 3> attachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

    GL_CALL(glDrawBuffers(3, attachments.data()));

    // Check if the creation failed
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Log::error("OpenGL::GBuffer", " could not initialize");
        return;
    }

    unbind();

    Log::debug("OpenGL::GBuffer", " created buffer");
}
}