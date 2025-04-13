#pragma once

// #include "Graphics/Images/Image2d.hpp"
#include "swapchain.hpp"

#include "utils/classes.hpp"

namespace xen {
class LogicalDevice;
class ImageDepth;
class Renderpass;
class RenderStage;

class XEN_API Framebuffers : NonCopyable {
private:
    LogicalDevice const& logical_device;

    // std::vector<std::unique_ptr<Image2d>> imageAttachments;
    std::vector<uint32_t> framebuffers;

public:
    constexpr Framebuffers(
        LogicalDevice const& logical_device, Swapchain const& swapchain, RenderStage const& render_stage,
        Renderpass const& render_pass, ImageDepth const& depth_stencil, Vector2ui const& extent, uint32_t samples = 0
    );
    ~Framebuffers();

    // Image2d* GetAttachment(uint32_t index) const { return imageAttachments[index].get(); }

    // std::vector<std::unique_ptr<Image2d>> const& GetImageAttachments() const { return imageAttachments; }
    [[nodiscard]] constexpr std::vector<uint32_t> const& get_framebuffers() const { return framebuffers; }
};
}