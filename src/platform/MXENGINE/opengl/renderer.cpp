#include "renderer.hpp"

#include <math.h>
#include "gl_utils.hpp"
#include "glm/glm.hpp"
#include "index_buffer.hpp"

// #include "Platform/Modules/GraphicModule.h"
// #include "Utilities/Profiler/Profiler.h"
// #include "Utilities/Format/Format.h"

#include "debug/log.hpp"

namespace xen {
constexpr std::array<GLenum, 14> BlendTable = {
    0x00000000,
    GL_ZERO,
    GL_ONE,
    GL_ONE_MINUS_SRC_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_CONSTANT_COLOR,
    GL_ONE_MINUS_CONSTANT_COLOR,
    GL_CONSTANT_ALPHA,
    GL_ONE_MINUS_CONSTANT_ALPHA,
};

constexpr std::array<GLenum, 12> PrimitiveTable = {
    GL_POINTS,
    GL_LINE_STRIP,
    GL_LINE_LOOP,
    GL_LINES,
    GL_LINE_STRIP_ADJACENCY,
    GL_LINES_ADJACENCY,
    GL_TRIANGLE_STRIP,
    GL_TRIANGLE_FAN,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP_ADJACENCY,
    GL_TRIANGLES_ADJACENCY,
    GL_PATCHES,
};

constexpr Renderer::Renderer()
{
    this->clear_mask |= GL_COLOR_BUFFER_BIT;
}

void Renderer::clear() const
{
    GL_CALL(glClear(clear_mask));
}

void Renderer::flush() const
{
    // MAKE_SCOPE_PROFILER("Renderer::Flush");

    // GraphicModule::OnRenderDraw();

    glFlush();
}

void Renderer::finish() const
{
    // MAKE_SCOPE_PROFILER("Renderer::Finish");

    // GraphicModule::OnRenderDraw();

    glFinish();
}

void Renderer::set_viewport(int x, int y, int width, int height) const
{
    GL_CALL(glViewport(x, y, width, height));
}

void Renderer::draw_vertices(RenderPrimitive primitive, size_t count, size_t offset)
{
    GL_CALL(glDrawArrays(
        PrimitiveTable[static_cast<size_t>(primitive)], static_cast<GLint>(offset), static_cast<GLsizei>(count)
    ));
}

void Renderer::draw_indices(RenderPrimitive primitive, size_t count, size_t offset)
{
    GL_CALL(glDrawElements(
        PrimitiveTable[(size_t)primitive], count, get_gl_type<IndexBuffer::IndexType>(),
        (void const*)(offset * sizeof(IndexBuffer::IndexType))
    ));
}

void Renderer::draw_vertices_instanced(
    RenderPrimitive primitive, size_t count, size_t offset, size_t instance_count, size_t instance_base
)
{
    GL_CALL(glDrawArraysInstancedBaseInstance(
        PrimitiveTable[static_cast<size_t>(primitive)], static_cast<GLint>(offset), static_cast<GLsizei>(count),
        instance_count, instance_base
    ));
}

void Renderer::draw_indices_instanced(
    RenderPrimitive primitive, size_t count, size_t offset, size_t instance_count, size_t instance_base
)
{
    GL_CALL(glDrawElementsInstancedBaseInstance(
        PrimitiveTable[static_cast<size_t>(primitive)], count, get_gl_type<IndexBuffer::IndexType>(),
        (void const*)(offset * sizeof(IndexBuffer::IndexType)), instance_count, instance_base
    ));
}

void Renderer::draw_indices_base_vertex(RenderPrimitive primitive, size_t count, size_t offset, size_t base)
{
    GL_CALL(glDrawElementsBaseVertex(
        PrimitiveTable[static_cast<size_t>(primitive)], count, get_gl_type<IndexBuffer::IndexType>(),
        (void const*)(offset * sizeof(IndexBuffer::IndexType)), base
    ));
}

void Renderer::draw_indices_base_vertex_instanced(
    RenderPrimitive primitive, size_t count, size_t offset, size_t base, size_t instance_count, size_t instance_base
)
{
    GL_CALL(glDrawElementsInstancedBaseVertexBaseInstance(
        PrimitiveTable[static_cast<size_t>(primitive)], count, get_gl_type<IndexBuffer::IndexType>(),
        (void const*)(offset * sizeof(IndexBuffer::IndexType)), instance_count, base, instance_base
    ));
}

Renderer& Renderer::use_clip_distance(size_t count)
{
    for (size_t i = 0; i < count; i++) {
        GL_CALL(glEnable(GL_CLIP_DISTANCE0 + i));
    }
    return *this;
}

Renderer& Renderer::use_seamless_cube_maps(bool value)
{
    if (value)
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    else
        glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    return *this;
}

Renderer& Renderer::use_color_mask(bool r, bool g, bool b, bool a)
{
    GL_CALL(glColorMask(r, g, b, a));
    return *this;
}

Renderer& Renderer::use_depth_buffer_mask(bool value)
{
    GL_CALL(glDepthMask(value));
    return *this;
}

Renderer& Renderer::use_sampling(bool value)
{
    if (value) {
        GL_CALL(glEnable(GL_MULTISAMPLE));
        Log::debug("OpenGL::Renderer", " native multisampling enabled");
    }
    else {
        GL_CALL(glDisable(GL_MULTISAMPLE));
        Log::debug("OpenGL::Renderer", " native multisampling disabled");
    }
    return *this;
}

Renderer& Renderer::use_depth_buffer(bool value)
{
    depth_buffer = value;
    if (value) {
        GL_CALL(glEnable(GL_DEPTH_TEST));
        clear_mask |= GL_DEPTH_BUFFER_BIT;
    }
    else {
        GL_CALL(glDisable(GL_DEPTH_TEST));
        clear_mask &= ~GL_DEPTH_BUFFER_BIT;
    }
    return *this;
}

Renderer& Renderer::use_depth_clamp(bool value)
{
    if (value) {
        GL_CALL(glEnable(GL_DEPTH_CLAMP));
    }
    else {
        GL_CALL(glDisable(GL_DEPTH_CLAMP));
    }
    return *this;
}

Renderer& Renderer::use_reversed_depth(bool value)
{
    if (value) {
        GL_CALL(glClearDepth(0.0f));
        GL_CALL(glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE));
        use_depth_function(DepthFunction::GREATER_EQUAL);
    }
    else {
        GL_CALL(glClearDepth(1.0f));
        GL_CALL(glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE));
        use_depth_function(DepthFunction::LESS);
    }
    return *this;
}

constexpr std::array<GLenum, 8> depth_funcs = {
    GL_EQUAL, GL_NOTEQUAL, GL_LESS, GL_GREATER, GL_LEQUAL, GL_GEQUAL, GL_ALWAYS, GL_NEVER,
};

Renderer& Renderer::use_depth_function(DepthFunction function)
{
    GL_CALL(glDepthFunc(depth_funcs[static_cast<size_t>(function)]));
    return *this;
}

Renderer& Renderer::use_culling(bool value, bool counter_clock_wise, bool cull_back)
{
    // culling
    if (value) {
        GL_CALL(glEnable(GL_CULL_FACE));
    }
    else {
        GL_CALL(glDisable(GL_CULL_FACE));
    }

    // point order
    if (counter_clock_wise) {
        GL_CALL(glFrontFace(GL_CCW));
    }
    else {
        GL_CALL(glFrontFace(GL_CW));
    }

    // back / front culling
    if (cull_back) {
        GL_CALL(glCullFace(GL_BACK));
    }
    else {
        GL_CALL(glCullFace(GL_FRONT));
    }

    return *this;
}

Renderer& Renderer::use_clear_color(float r, float g, float b, float a)
{
    GL_CALL(glClearColor(r, g, b, a));
    return *this;
}

Renderer& Renderer::use_blend_factors(BlendFactor src, BlendFactor dist)
{
    if (src == BlendFactor::NONE || dist == BlendFactor::NONE) {
        GL_CALL(glDisable(GL_BLEND));
    }
    else {
        GL_CALL(glEnable(GL_BLEND));
        GL_CALL(glBlendFunc(BlendTable[static_cast<size_t>(src)], BlendTable[static_cast<size_t>(dist)]));
    }
    return *this;
}

Renderer& Renderer::use_anisotropic_filtering(float factor)
{
    if (!glfwExtensionSupported("GL_EXT_texture_filter_anisotropic")) {
        Log::warning("OpenGL::Renderer", " anisotropic filtering is not supported on your device");
    }
    else {
        GL_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, factor));
        Log::debug("OpenGL::Renderer", " set anisotropic filtering factor to ", factor, "x");
    }
    return *this;
}

constexpr float Renderer::get_largest_anisotropic_factor() const
{
    if (!glfwExtensionSupported("GL_EXT_texture_filter_anisotropic")) {
        Log::warning("OpenGL::Renderer", " anisotropic filtering is not supported on your device");
        return 0.0f;
    }
    float factor = 0.f;
    GL_CALL(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &factor));
    return factor;
}

void Renderer::default_vertex_attribute(size_t index, float v)
{
    GL_CALL(glVertexAttrib1f(static_cast<GLuint>(index), v));
}

void Renderer::default_vertex_attribute(size_t index, Vector2f const& vec)
{
    GL_CALL(glVertexAttrib2f(static_cast<GLuint>(index), vec.x, vec.y));
}

void Renderer::default_vertex_attribute(size_t index, Vector3f const& vec)
{
    GL_CALL(glVertexAttrib3f(static_cast<GLuint>(index), vec.x, vec.y, vec.z));
}

void Renderer::default_vertex_attribute(size_t index, Vector4f const& vec)
{
    GL_CALL(glVertexAttrib4f(static_cast<GLuint>(index), vec.x, vec.y, vec.z, vec.w));
}

void Renderer::default_vertex_attribute(size_t index, Matrix2 const& mat)
{
    for (size_t i = 0; i < Matrix2::COUNT; i++) {
        default_vertex_attribute(index + i, mat[i]);
    }
}

void Renderer::default_vertex_attribute(size_t index, Matrix3 const& mat)
{
    for (size_t i = 0; i < Matrix3::COUNT; i++) {
        default_vertex_attribute(index + i, mat[i]);
    }
}

void Renderer::default_vertex_attribute(size_t index, Matrix4 const& mat)
{
    for (size_t i = 0; i < Matrix4::COUNT; i++) {
        default_vertex_attribute(index + i, mat[i]);
    }
}
}