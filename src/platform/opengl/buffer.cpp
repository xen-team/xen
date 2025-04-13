#include "buffer.hpp"

namespace xen {
Buffer::Buffer()
{
    GL_CALL(glGenBuffers(1, &id));
}

// Can be used to allocate a big buffer then you can use SetData to vary its size
Buffer::Buffer(uint32_t const byte_size, BufferType const type, UsageType const usage) : type{type}, usage{usage}
{
    GL_CALL(glGenBuffers(1, &id));

    bind();
    GL_CALL(glBufferData(static_cast<GLenum>(type), byte_size, nullptr, GL_DYNAMIC_DRAW)
    ); // Dynamic since this will change size and what were drawing

    Log::debug("OpenGL::Buffer", " created buffer with id: ", this->id);
}

Buffer::Buffer(uint8_t* data, size_t const byte_size, BufferType const type, UsageType const usage) :
    type{type}, usage{usage}
{
    GL_CALL(glGenBuffers(1, &id));
    load(data, byte_size, type, usage);

    Log::debug("OpenGL::Buffer", " created buffer with id: ", this->id);
}

Buffer::~Buffer()
{
    if (id != 0) {
        GL_CALL(glDeleteBuffers(1, &id));

        Log::debug("OpenGL::Buffer", " deleted buffer with id: ", id);

        id = 0;
    }
}

void Buffer::set_data(void const* data, uint32_t byte_size)
{
    bind();
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, byte_size, data));
}

void Buffer::load(uint8_t const* data, size_t byte_size, BufferType type, UsageType usage)
{
    this->type = type;
    this->usage = usage;

    bind();
    GL_CALL(
        glBufferData(static_cast<GLenum>(type), static_cast<GLsizeiptr>(byte_size), data, static_cast<GLenum>(usage))
    );
}

void Buffer::buffer_sub_data(uint8_t const* data, size_t byte_size, size_t offset)
{
    assert(byte_size + offset <= this->byte_size);

    bind();

    GL_CALL(glBufferSubData(static_cast<GLenum>(this->type), offset, byte_size, data));
}

void Buffer::buffer_data_with_resize(uint8_t const* data, size_t byte_size)
{
    if (this->byte_size < byte_size) {
        load(data, byte_size, type, usage);
    }
    else {
        buffer_sub_data(data, byte_size);
    }
}

void Buffer::get_buffer_data(uint8_t* byte_data, size_t byte_size, size_t offset) const
{
    assert(byte_size + offset <= this->byte_size);

    bind();

    GL_CALL(glGetBufferSubData(static_cast<GLenum>(this->type), offset, byte_size, byte_data));
}

void Buffer::bind() const
{
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, id));
}

void Buffer::unbind() const
{
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void Buffer::bind_base(size_t index) const
{
    GL_CALL(glBindBufferBase(static_cast<GLenum>(type), index, id));
}
}