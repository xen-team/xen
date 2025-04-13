#include "framebuffers.hpp"

#include <glad/glad.h>

// #include "Graphics/Images/ImageDepth.hpp"
#include "renderpass.hpp"
#include "render/render.hpp"
#include "render/render_stage.hpp"

namespace xen {
constexpr Framebuffers::Framebuffers(
    LogicalDevice const& logical_device, Swapchain const& swapchain, RenderStage const& render_stage,
    Renderpass const& render_pass, ImageDepth const& depth_stencil, Vector2ui const& extent, uint32_t samples
) : logical_device(logical_device)
{
    for (auto const& attachment : render_stage.get_attachments()) {
        uint32_t attachment_samples = attachment.is_multisampled() ? samples : 0;

        /*
        switch (attachment.get_type()) {
        case Attachment::Type::Image:
            image_attachments.emplace_back(std::make_unique<Image2d>(
                extent, attachment.GetFormat(), GL_LINEAR, GL_CLAMP_TO_EDGE, attachmentSamples
            ));
            break;
        case Attachment::Type::Depth:
            image_attachments.emplace_back(nullptr);
            break;
        case Attachment::Type::Swapchain:
            image_attachments.emplace_back(nullptr);
            break;
        }
        */
    }

    framebuffers.resize(2);

    for (uint32_t i = 0; i < 2; i++) {
        std::vector<GLuint> attachments;

        /*
        for (auto const& attachment : render_stage.get_attachments()) {
            switch (attachment.get_type()) {
            case Attachment::Type::Image:
                attachments.emplace_back(get_attachment(attachment.get_binding())->get_texture_id());
                break;
            case Attachment::Type::Depth:
                attachments.emplace_back(depth_stencil.get_texture_id());
                break;
            case Attachment::Type::Swapchain:
                attachments.emplace_back(swapchain.get_texture_id().at(i));
                break;
            }
        }
        */

        uint32_t framebuffer = 0;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        for (uint32_t j = 0; j < attachments.size(); j++) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j, GL_TEXTURE_2D, attachments[j], 0);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            // Handle framebuffer incomplete error
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        framebuffers[i] = framebuffer;
    }
}

Framebuffers::~Framebuffers()
{
    for (auto const& framebuffer : framebuffers) {
        glDeleteFramebuffers(1, &framebuffer);
    }
}
}
