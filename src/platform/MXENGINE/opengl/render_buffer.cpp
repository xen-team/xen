#include "render_buffer.hpp"

#include "platform/opengl/gl_utils.hpp"
#include "platform/opengl/framebuffers.hpp"
#include "debug/log.hpp"

namespace xen {
void RenderBuffer::free()
{
    if (id != 0) {
        GL_CALL(glDeleteRenderbuffers(1, &id));
        Log::debug("OpenGL::RenderBuffer", " freed renderbuffer with id: ", id);
    }
    id = 0;
}

RenderBuffer::RenderBuffer()
{
    GL_CALL(glGenRenderbuffers(1, &id));
    Log::debug("OpenGL::RenderBuffer", " created renderbuffer with id: ", id);
}

RenderBuffer::~RenderBuffer()
{
    free();
}

RenderBuffer::RenderBuffer(RenderBuffer&& renderbuffer) noexcept :
    id(renderbuffer.id), width(renderbuffer.width), height(renderbuffer.height), samples(renderbuffer.samples)
{
    renderbuffer.id = 0;
    renderbuffer.width = 0;
    renderbuffer.height = 0;
    renderbuffer.samples = 0;
}

RenderBuffer& RenderBuffer::operator=(RenderBuffer&& renderbuffer) noexcept
{
    free(); //-V509

    id = renderbuffer.id;
    width = renderbuffer.width;
    height = renderbuffer.height;
    samples = renderbuffer.samples;

    renderbuffer.id = 0;
    renderbuffer.width = 0;
    renderbuffer.height = 0;
    renderbuffer.samples = 0;
    return *this;
}

void RenderBuffer::bind() const
{
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, this->id));
}

void RenderBuffer::unbind() const
{
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

void RenderBuffer::init_storage(int width, int height, int samples)
{
    assert(samples >= 0 && width >= 0 && height >= 0);
    this->width = width;
    this->height = height;
    this->samples = samples;

    bind();
    GL_CALL(glRenderbufferStorageMultisample(
        GL_RENDERBUFFER, static_cast<GLsizei>(samples), GL_DEPTH_STENCIL, static_cast<GLsizei>(width),
        static_cast<GLsizei>(height)
    ));
}

void RenderBuffer::link_to_framebuffer(FrameBuffer const& framebuffer) const
{
    framebuffer.bind();
    GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->id));
    framebuffer.unbind();
}
}