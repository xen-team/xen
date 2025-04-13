#include "renderpass.hpp"

#include <glad/glad.h>

#include "render/render.hpp"
#include "render/render_stage.hpp"

namespace xen {
Renderpass::Renderpass(
    RenderStage const& render_stage, uint32_t depth_format,
    uint32_t surface_format, uint32_t samples
)
{
    std::vector<uint32_t> framebuffers;
    framebuffers.reserve(render_stage.get_subpasses().size());

    for (auto const& subpassType : render_stage.get_subpasses()) {
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        auto window_size = Windows::get()->get_window(0)->get_size();

        // Create texture for color attachment
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, surface_format, window_size.x, window_size.y, 0, surface_format, GL_UNSIGNED_BYTE, nullptr
        );
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        // Create renderbuffer for depth attachment
        GLuint renderbuffer;
        glGenRenderbuffers(1, &renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, depth_format, window_size.x, window_size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            // Handle error
        }

        framebuffers.emplace_back(framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Store the framebuffers
    this->framebuffers = framebuffers;
}

Renderpass::~Renderpass()
{
    for (auto const& framebuffer : framebuffers) {
        glDeleteFramebuffers(1, &framebuffer);
    }
}

void Renderpass::bind(uint32_t subpass_index)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[subpass_index]);
}

void Renderpass::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
}