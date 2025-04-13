#include "buffer_base.hpp"
#include "gl_utils.hpp"
#include "debug/log.hpp"

namespace xen {
void BufferBase::free()
{
    if (this->id != 0) {
        GL_CALL(glDeleteBuffers(1, &this->id));

        Log::debug("OpenGL::Buffer", " freed buffer with id: ", this->id);

        this->id = 0;
    }
}

BufferBase::BufferBase()
{
    GL_CALL(glGenBuffers(1, &this->id));

    Log::debug("OpenGL::Buffer", " created buffer with id: ", this->id);
}

BufferBase::~BufferBase()
{
    this->free();
}

BufferBase::BufferBase(BufferBase&& other) noexcept :
    id(other.id), byte_size(other.byte_size), type(other.type), usage(other.usage)
{
}

BufferBase& BufferBase::operator=(BufferBase&& other) noexcept
{
    free();

    id = other.id;
    byte_size = other.byte_size;
    type = other.type;
    usage = other.usage;

    return *this;
}

void BufferBase::bind() const
{
    GL_CALL(glBindBuffer(static_cast<GLenum>(type), id));
}

void BufferBase::unbind() const
{
    GL_CALL(glBindBuffer(static_cast<GLenum>(type), 0));
}

void BufferBase::bind_base(size_t index) const
{
    GL_CALL(glBindBufferBase(static_cast<GLenum>(type), index, id));
}

void BufferBase::load_from(BufferBase& other)
{
    assert(other.byte_size <= byte_size);

    GL_CALL(glBindBuffer(GL_COPY_READ_BUFFER, other.id));
    GL_CALL(glBindBuffer(GL_COPY_WRITE_BUFFER, id));
    GL_CALL(glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, other.byte_size));
}

void BufferBase::load(BufferType type, uint8_t const* byte_data, size_t byte_size, UsageType usage)
{
    this->type = type;
    this->byte_size = byte_size;
    this->usage = usage;
    bind();

    GL_CALL(glBufferData(static_cast<GLenum>(this->type), this->byte_size, byte_data, static_cast<GLenum>(this->usage))
    );
}

void BufferBase::buffer_sub_data(uint8_t const* byte_data, size_t byte_size, size_t offset)
{
    assert(byte_size + offset <= this->byte_size);

    this->bind();

    GL_CALL(glBufferSubData(static_cast<GLenum>(this->type), offset, byte_size, byte_data));
}

void BufferBase::buffer_data_with_resize(uint8_t const* byte_data, size_t byte_size)
{
    if (this->byte_size < byte_size) {
        load(this->type, byte_data, byte_size, this->usage);
    }
    else {
        buffer_sub_data(byte_data, byte_size);
    }
}

void BufferBase::get_buffer_data(uint8_t* byte_data, size_t byte_size, size_t offset) const
{
    assert(byte_size + offset <= this->byte_size);

    bind();

    GL_CALL(glGetBufferSubData(static_cast<GLenum>(this->type), offset, byte_size, byte_data));
}
}