#pragma once

#include "platform/opengl/texture.hpp"
#include "platform/opengl/cubemap.hpp"
#include "platform/opengl/texture.hpp"
#include "utils/classes.hpp"

namespace xen {
enum class Attachment : uint8_t {
    COLOR_ATTACHMENT0,
    COLOR_ATTACHMENT1,
    COLOR_ATTACHMENT2,
    COLOR_ATTACHMENT3,
    COLOR_ATTACHMENT4,
    COLOR_ATTACHMENT5,
    COLOR_ATTACHMENT6,
    COLOR_ATTACHMENT9,
    COLOR_ATTACHMENT10,
    COLOR_ATTACHMENT11,
    COLOR_ATTACHMENT12,
    COLOR_ATTACHMENT13,
    COLOR_ATTACHMENT14,
    COLOR_ATTACHMENT15,
    DEPTH_ATTACHMENT,
    STENCIL_ATTACHMENT,
    DEPTH_STENCIL_ATTACHMENT,
};

using FrameBufferHandle = uint32_t;

class FrameBuffers : NonCopyable {
    enum class AttachmentType : uint8_t {
        NONE,
        TEXTURE,
        CUBEMAP,
    };

    using BindableId = unsigned int;

    BindableId id = 0;
    std::vector<std::unique_ptr<Image>> image_attachments;
    std::vector<std::unique_ptr<Image>> cubemap_attachments;
    std::vector<uint32_t> framebuffers;

    // #if defined(XEN_DEBUG)
    //     mutable const Texture* _texturePtr = nullptr;
    //     mutable CubeMap const* _cubemapPtr = nullptr;
    // #endif

public:
    FrameBuffers(Vector2ui const& extent) {};

    void validate() const;

    void use_draw_buffers(std::span<Attachment> attachments) const;

    void use_only_depth() const;

    [[nodiscard]] size_t get_width() const;
    [[nodiscard]] size_t get_height() const;

    void bind() const;
    void unbind() const;

    [[nodiscard]] BindableId get_native_handle() const { return id; }

private:
    void on_texture_attach(Texture const& texture, Attachment attachment);
    void on_cubemap_attach(CubeMap const& cubemap, Attachment attachment);
    void free();
};
}