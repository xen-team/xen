#pragma once

#include <optional>
#include <vector>

#include "utils/classes.hpp"
#include "core.hpp"

namespace xen {
class LogicalDevice;
class ImageDepth;
class RenderStage;

class XEN_API Renderpass {
private:
    std::vector<uint32_t> framebuffers;
    uint32_t renderbuffer;
    uint32_t texture;

public:
    class SubpassDescription : NonCopyable {
    private:
        uint32_t draw_buffer;
        std::vector<uint32_t> color_attachments;
        std::optional<uint32_t> depth_attachment;

    public:
        constexpr SubpassDescription(
            uint32_t draw_buffer, std::vector<uint32_t> color_attachments,
            std::optional<uint32_t> const& depth_attachment
        ) :
            draw_buffer(draw_buffer), color_attachments(std::move(color_attachments)),
            depth_attachment(depth_attachment)
        {
        }

        [[nodiscard]] constexpr uint32_t get_draw_buffer() const { return draw_buffer; }
        [[nodiscard]] constexpr std::vector<uint32_t> get_color_attachments() const { return color_attachments; }
        [[nodiscard]] constexpr std::optional<uint32_t> get_depth_attachment() const { return depth_attachment; }
    };

    Renderpass(
        RenderStage const& renderStage, uint32_t depth_format,
        uint32_t surface_format, uint32_t samples = 0
    );
    ~Renderpass();

    void bind(uint32_t subpass_index);
    void unbind();
};
}