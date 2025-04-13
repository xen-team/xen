#include "framebuffers.hpp"

#include "platform/opengl/gl_utils.hpp"
#include "debug/log.hpp"

#include <array>

// bind unbind

namespace xen {
constexpr std::array<GLenum, 17> attachment_table{
    GL_COLOR_ATTACHMENT0,        GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT2,  GL_COLOR_ATTACHMENT3,
    GL_COLOR_ATTACHMENT4,        GL_COLOR_ATTACHMENT5,  GL_COLOR_ATTACHMENT6,  GL_COLOR_ATTACHMENT9,
    GL_COLOR_ATTACHMENT10,       GL_COLOR_ATTACHMENT11, GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13,
    GL_COLOR_ATTACHMENT14,       GL_COLOR_ATTACHMENT15, GL_DEPTH_ATTACHMENT,   GL_STENCIL_ATTACHMENT,
    GL_DEPTH_STENCIL_ATTACHMENT,
};

FrameBuffer::FrameBuffer()
{
    GL_CALL(glGenFramebuffers(1, &id));
    Log::debug("OpenGL::FrameBuffer", " created framebuffer with id: ", id);
}

void FrameBuffer::on_texture_attach(Texture const& texture, Attachment attachment)
{
    GLenum const mode = attachment_table[static_cast<int>(attachment)];
    GLint const texture_id = texture.get_native_handle();

    bind();
    GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, mode, texture.get_texture_type(), texture_id, 0));
}

void FrameBuffer::on_cubemap_attach(CubeMap const& cubemap, Attachment attachment)
{
    GLenum const mode = attachment_table[static_cast<int>(attachment)];
    GLint const cubemap_id = cubemap.get_native_handle();

    bind();
    GL_CALL(glFramebufferTexture(GL_FRAMEBUFFER, mode, cubemap_id, 0));
}

void FrameBuffer::free()
{
    detach_render_target();
    if (id != 0) {
        GL_CALL(glDeleteFramebuffers(1, &id));
        Log::debug("OpenGL::FrameBuffer", " freed framebuffer with id: ", id);
    }
    id = 0;
}

void FrameBuffer::copy_framebuffer_contents(int width, int height) const
{
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, this->id));
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
    GL_CALL(glBlitFramebuffer(
        0, 0, static_cast<GLint>(get_width()), static_cast<GLint>(this->get_height()), 0, 0, width, height,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    ));
}

void FrameBuffer::validate() const
{
    bind();
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Log::error("OpenGL::FrameBuffer", " framebuffer validation failed: incomplete");
    }
}

void FrameBuffer::detach_render_target()
{
    if (current_attachment == AttachmentType::TEXTURE)
        std::launder(reinterpret_cast<TextureHandle*>(&attachment_storage))->~Resource();
    else if (current_attachment == AttachmentType::CUBEMAP)
        std::launder(reinterpret_cast<CubeMapHandle*>(&attachment_storage))->~Resource();

    current_attachment = AttachmentType::NONE;

    // #if defined(MXENGINE_DEBUG)
    //     this->_texturePtr = nullptr;
    //     this->_cubemapPtr = nullptr;
    // #endif
}

void FrameBuffer::detach_extra_target(Attachment attachment)
{
    GL_CALL(glFramebufferTexture(GL_FRAMEBUFFER, attachment_table[static_cast<int>(attachment)], 0, 0));
}

bool FrameBuffer::has_texture_attached() const
{
    return current_attachment == AttachmentType::TEXTURE;
}

bool FrameBuffer::has_cubemap_attached() const
{
    return current_attachment == AttachmentType::CUBEMAP;
}

void FrameBuffer::use_draw_buffers(std::span<Attachment> attachments) const
{
    std::array<GLenum, 20> attachment_types{};
    assert(attachments.size() <= attachmentTypes.size());

    bind();
    for (size_t i = 0; i < attachments.size(); i++) {
        attachment_types[i] = attachment_table[static_cast<int>(attachments[i])];
    }
    GL_CALL(glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachment_types.data()));
}

void FrameBuffer::use_only_depth() const
{
    bind();
    GL_CALL(glDrawBuffer(GL_NONE));
}

size_t FrameBuffer::get_width() const
{
    auto* texture = reinterpret_cast<Resource<Texture, TextureFactory> const*>(&this->attachmentStorage);
    auto* cubemap = reinterpret_cast<Resource<CubeMap, CubeMapFactory> const*>(&this->attachmentStorage);

    if (this->currentAttachment == AttachmentType::TEXTURE && std::launder(texture)->is_valid())
        return (*texture)->GetWidth();
    if (this->currentAttachment == AttachmentType::CUBEMAP && std::launder(cubemap)->IsValid())
        return (*cubemap)->GetWidth();

    return 0;
}

size_t FrameBuffer::get_height() const
{
    auto* texture = reinterpret_cast<Resource<Texture, TextureFactory> const*>(&this->attachmentStorage);
    auto* cubemap = reinterpret_cast<Resource<CubeMap, CubeMapFactory> const*>(&this->attachmentStorage);

    if (this->currentAttachment == AttachmentType::TEXTURE && std::launder(texture)->IsValid())
        return (*std::launder(texture))->GetHeight();
    if (this->currentAttachment == AttachmentType::CUBEMAP && std::launder(cubemap)->IsValid())
        return (*std::launder(cubemap))->GetHeight();

    return 0;
}

FrameBuffer::~FrameBuffer()
{
    this->FreeFrameBuffer();
}

FrameBuffer::FrameBuffer(FrameBuffer&& framebuffer) noexcept
{
    this->id = framebuffer.id;
    this->currentAttachment = framebuffer.currentAttachment;
    this->attachmentStorage = framebuffer.attachmentStorage;
    framebuffer.currentAttachment = AttachmentType::NONE;
    framebuffer.id = 0;
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& framebuffer) noexcept
{
    this->FreeFrameBuffer();

    this->id = framebuffer.id;
    this->currentAttachment = framebuffer.currentAttachment;
    this->attachmentStorage = framebuffer.attachmentStorage;
    framebuffer.currentAttachment = AttachmentType::NONE;
    framebuffer.id = 0;
    return *this;
}

void FrameBuffer::Bind() const
{
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, id));
}

void FrameBuffer::Unbind() const
{
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

FrameBuffer::BindableId FrameBuffer::GetNativeHandle() const
{
    return id;
}

void FrameBuffer::CopyFrameBufferContents(FrameBuffer const& framebuffer) const
{
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, this->id));
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.GetNativeHandle()));
    GL_CALL(glBlitFramebuffer(
        0, 0, (GLint)this->GetWidth(), (GLint)this->GetHeight(), 0, 0, (GLint)framebuffer.GetWidth(),
        (GLint)framebuffer.GetHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST
    ));
}
}