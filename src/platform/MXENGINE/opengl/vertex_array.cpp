#include "vertex_array.hpp"
#include "gl_utils.hpp"
#include "vertex_buffer.hpp"
#include "index_buffer.hpp"
#include "vertex_attribute.hpp"
#include "debug/log.hpp"

namespace xen {
void VertexArray::free()
{
    if (id != 0) {
        GL_CALL(glDeleteVertexArrays(1, &id));

        Log::debug("OpenGL::VertexArray", " freed vertex array with id: ", id);
    }
    id = 0;
}

VertexArray::VertexArray()
{
    GL_CALL(glGenVertexArrays(1, &id));

    Log::debug("OpenGL::VertexArray", " created vertex array with id: ", id);
}

VertexArray::~VertexArray()
{
    free();
}

VertexArray::VertexArray(VertexArray&& array) noexcept : id(array.id), attribute_index(array.attribute_index) {}

VertexArray& VertexArray::operator=(VertexArray&& array) noexcept
{
    free();
    attribute_index = array.attribute_index;
    id = array.id;
    return *this;
}

void VertexArray::bind() const
{
    glBindVertexArray(id);
}

void VertexArray::unbind() const
{
    glBindVertexArray(0);
}

void VertexArray::add_vertex_layout(
    VertexBuffer const& buffer, std::span<VertexAttribute> layout, VertexAttributeInputRate input_rate
)
{
    bind();
    buffer.bind();

    size_t offset = 0;
    size_t stride = 0;

    for (auto const& element : layout) {
        stride += element.byte_size;
    }

    for (auto const& element : layout) {
        for (size_t i = 0; i < element.entries; i++) {
            // TODO: handle integer case with glVertexAttribIPointer
            GL_CALL(glEnableVertexAttribArray(attribute_index));
            GL_CALL(glVertexAttribPointer(
                attribute_index, element.components, static_cast<GLenum>(element.type), GL_FALSE, stride, (void*)offset
            ));
            if (input_rate == VertexAttributeInputRate::PER_INSTANCE) {
                GL_CALL(glVertexAttribDivisor(attribute_index, 1));
            }

            offset += element.byte_size / element.entries;
            attribute_index++;
        }
    }
    unbind();
}

void VertexArray::remove_vertex_layout(std::span<VertexAttribute> layout)
{
    assert(attribute_index > layout.size());

    bind();
    for (auto const& element : layout) {
        for (size_t i = 0; i < element.entries; i++) {
            attribute_index--;

            GL_CALL(glDisableVertexAttribArray(attribute_index));
        }
    }
    unbind();
}

void VertexArray::link_index_buffer(IndexBuffer const& buffer)
{
    bind();
    buffer.bind();
    unbind();
}
}