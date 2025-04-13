#pragma once

#include "renderpass/renderpass.hpp"
#include "renderpass/framebuffers.hpp"

#include "utils/color.hpp"
#include "math/vector2.hpp"

#include <GL/gl.h>

#include <utility>

namespace xen {
class XEN_API Attachment {
public:
    enum class Type { Image, Depth, Swapchain };

private:
    uint32_t binding;
    std::string name;
    Type type;
    bool multisampled;
    GLenum format;
    Color clear_color;

public:
    constexpr Attachment(
        uint32_t binding, std::string name, Type type, bool multisampled = false, GLenum format = GL_RGBA,
        Color const& clear_color = Color::black
    ) :
        binding(binding), name(std::move(name)), type(type), multisampled(multisampled), format(format),
        clear_color(clear_color)
    {
    }

    [[nodiscard]] constexpr uint32_t get_binding() const { return binding; }
    [[nodiscard]] constexpr std::string const& get_name() const { return name; }
    [[nodiscard]] constexpr Type get_type() const { return type; }
    [[nodiscard]] constexpr bool is_multisampled() const { return multisampled; }
    [[nodiscard]] constexpr GLenum get_format() const { return format; }
    [[nodiscard]] constexpr Color const& get_clear_color() const { return clear_color; }
};

class XEN_API SubpassType {
private:
    uint32_t binding;
    std::vector<uint32_t> attachment_bindings;

public:
    constexpr SubpassType(uint32_t binding, std::vector<uint32_t> attachment_bindings) :
        binding(binding), attachment_bindings(std::move(attachment_bindings))
    {
    }

    [[nodiscard]] constexpr uint32_t get_binding() const { return binding; }
    [[nodiscard]] constexpr std::vector<uint32_t> const& get_attachment_bindings() const { return attachment_bindings; }
};

class XEN_API RenderArea {
private:
    Vector2ui extent;
    Vector2i offset;
    float aspect_ratio = 1.0f;

public:
    constexpr explicit RenderArea(Vector2ui extent = {}, Vector2i offset = {}) :
        extent(std::move(extent)), offset(std::move(offset))
    {
    }

    [[nodiscard]] constexpr Vector2ui const& get_extent() const { return extent; }
    constexpr void set_extent(Vector2ui const& extent) { this->extent = extent; }

    [[nodiscard]] constexpr Vector2i const& get_offset() const { return offset; }
    constexpr void set_offset(Vector2i const& offset) { this->offset = offset; }

    [[nodiscard]] constexpr float get_aspect_ratio() const { return aspect_ratio; }
    constexpr void set_aspect_ratio(float aspect_ratio) { this->aspect_ratio = aspect_ratio; }

    bool operator==(RenderArea const& rhs) const { return extent == rhs.extent && offset == rhs.offset; }

    bool operator!=(RenderArea const& rhs) const { return !operator==(rhs); }
};

class XEN_API Viewport {
private:
    Vector2f scale = {1.0f, 1.0f};
    std::optional<Vector2ui> size;
    Vector2i offset;

public:
    constexpr Viewport() = default;

    constexpr explicit Viewport(Vector2ui const& size) : size(size) {}

    [[nodiscard]] constexpr Vector2f const& get_scale() const { return scale; }
    constexpr void set_scale(Vector2f const& scale) { this->scale = scale; }

    [[nodiscard]] constexpr std::optional<Vector2ui> const& get_size() const { return size; }
    constexpr void set_size(std::optional<Vector2ui> const& size) { this->size = size; }

    [[nodiscard]] constexpr Vector2i const& get_offset() const { return offset; }
    constexpr void set_offset(Vector2i const& offset) { this->offset = offset; }
};

class XEN_API RenderStage {
    friend class Graphics;

private:
    using ClearValueType = std::array<GLfloat, 4>;

    std::vector<Attachment> attachments;
    std::vector<SubpassType> subpasses;

    Viewport viewport;

    std::unique_ptr<Renderpass> renderpass;
    std::unique_ptr<ImageDepth> depth_stencil;
    std::unique_ptr<Framebuffers> framebuffers;

    std::map<std::string, Descriptor const*> descriptors;

    std::vector<ClearValueType> clear_values;
    std::vector<uint32_t> subpass_attachment_count;
    std::optional<Attachment> depth_attachment;
    std::optional<Attachment> swapchain_attachment;
    std::vector<bool> subpass_multisampled;

    RenderArea render_area;
    bool out_of_date = false;

public:
    explicit RenderStage(
        std::vector<Attachment> images = {}, std::vector<SubpassType> subpasses = {},
        Viewport const& viewport = Viewport()
    );

    void update();
    void rebuild(Swapchain const& swapchain);

    std::optional<Attachment> get_attachment(std::string const& name) const;
    std::optional<Attachment> get_attachment(uint32_t binding) const;

    GLuint const& get_active_framebuffer(uint32_t active_swapchain_image) const;

    std::vector<Attachment> const& get_attachments() const { return attachments; }
    std::vector<SubpassType> const& get_subpasses() const { return subpasses; }

    Viewport& get_viewport() { return viewport; }
    void set_viewport(Viewport const& viewport) { this->viewport = viewport; }

    RenderArea const& get_render_area() const { return render_area; }

    bool is_out_of_date() const { return out_of_date; }

    Renderpass const* get_renderpass() const { return renderpass.get(); }
    ImageDepth const* get_depth_stencil() const { return depth_stencil.get(); }
    Framebuffers const* get_framebuffers() const { return framebuffers.get(); }
    std::vector<ClearValueType> const& get_clear_values() const { return clear_values; }
    uint32_t get_attachment_count(uint32_t subpass) const { return subpass_attachment_count[subpass]; }
    bool has_depth() const { return depth_attachment.has_value(); }
    bool has_swapchain() const { return swapchain_attachment.has_value(); }
    bool is_multisampled(uint32_t subpass) const { return subpass_multisampled[subpass]; }
};
}