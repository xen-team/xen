#include "vertex_attribute.hpp"
#include "gl_utils.hpp"

#include "math/matrix2.hpp"
#include "math/matrix3.hpp"
#include "math/vector4.hpp"
#include "math/matrix4.hpp"

namespace xen {
template <>
VertexAttribute VertexAttribute::entry<float>()
{
    return {.type = GL_FLOAT, .components = 1, .entries = 1, .byte_size = sizeof(float)};
}

template <>
VertexAttribute VertexAttribute::entry<int32_t>()
{
    return {.type = GL_INT, .components = 1, .entries = 1, .byte_size = sizeof(int32_t)};
}

template <>
VertexAttribute VertexAttribute::entry<Vector2i>()
{
    return {.type = GL_INT, .components = 2, .entries = 1, .byte_size = sizeof(Vector2i)};
}

template <>
VertexAttribute VertexAttribute::entry<Vector3i>()
{
    return {.type = GL_INT, .components = 3, .entries = 1, .byte_size = sizeof(Vector3i)};
}

template <>
VertexAttribute VertexAttribute::entry<Vector4i>()
{
    return {.type = GL_INT, .components = 4, .entries = 1, .byte_size = sizeof(Vector4i)};
}

template <>
VertexAttribute VertexAttribute::entry<Vector2f>()
{
    return {.type = GL_FLOAT, .components = 2, .entries = 1, .byte_size = sizeof(Vector2f)};
}

template <>
VertexAttribute VertexAttribute::entry<Vector3f>()
{
    return {.type = GL_FLOAT, .components = 3, .entries = 1, .byte_size = sizeof(Vector3f)};
}

template <>
VertexAttribute VertexAttribute::entry<Vector4f>()
{
    return {.type = GL_FLOAT, .components = 4, .entries = 1, .byte_size = sizeof(Vector4f)};
}

template <>
VertexAttribute VertexAttribute::entry<Matrix2>()
{
    return {.type = GL_FLOAT, .components = 2, .entries = 2, .byte_size = sizeof(Matrix2)};
}

template <>
VertexAttribute VertexAttribute::entry<Matrix3>()
{
    return {.type = GL_FLOAT, .components = 3, .entries = 3, .byte_size = sizeof(Matrix3)};
}

template <>
VertexAttribute VertexAttribute::entry<Matrix4>()
{
    return {.type = GL_FLOAT, .components = 4, .entries = 4, .byte_size = sizeof(Matrix4)};
}
}