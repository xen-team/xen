#include "submesh_renderer.hpp"

#include <render/renderer.hpp>

#include <tracy/Tracy.hpp>
#include <GL/glew.h> // Needed by TracyOpenGL.hpp
#include <tracy/TracyOpenGL.hpp>

namespace xen {
void SubmeshRenderer::set_render_mode(RenderMode render_mode, Submesh const& submesh)
{
    ZoneScopedN("SubmeshRenderer::set_render_mode");

    this->render_mode = render_mode;

    switch (render_mode) {
    case RenderMode::POINT:
        render_func = [](VertexBuffer const& vertex_buffer, IndexBuffer const&) {
            Renderer::draw_arrays(PrimitiveType::POINTS, vertex_buffer.vertex_count);
        };
        break;
    case RenderMode::LINE: {
        render_func = [](VertexBuffer const&, IndexBuffer const& index_buffer) {
            Renderer::draw_elements(PrimitiveType::LINES, index_buffer.line_index_count);
        };
        break;
    }
    case RenderMode::TRIANGLE:
    default:
        render_func = [](VertexBuffer const&, IndexBuffer const& index_buffer) {
            Renderer::draw_elements(PrimitiveType::TRIANGLES, index_buffer.triangle_index_count);
        };
        break;

#if !defined(USE_OPENGL_ES)
    case RenderMode::PATCH:
        render_func = [](VertexBuffer const& vertex_buffer, IndexBuffer const&) {
            Renderer::draw_arrays(PrimitiveType::PATCHES, vertex_buffer.vertex_count);
        };
        Renderer::set_patch_vertex_count(3); // Should be the default, but just in case
        break;
#endif
    }

    load_indices(submesh);
}

SubmeshRenderer SubmeshRenderer::clone() const
{
    SubmeshRenderer submesh_renderer;

    submesh_renderer.render_mode = render_mode;
    submesh_renderer.render_func = render_func;
    submesh_renderer.material_index = material_index;

    return submesh_renderer;
}

void SubmeshRenderer::load(Submesh const& submesh, RenderMode render_mode)
{
    ZoneScopedN("SubmeshRenderer::load");

    load_vertices(submesh);
    set_render_mode(render_mode, submesh);
}

void SubmeshRenderer::draw() const
{
    ZoneScopedN("SubmeshRenderer::draw");
    TracyGpuZone("SubmeshRenderer::draw")

        vao.bind();
    ibo.bind();

    render_func(vbo, ibo);
}

void SubmeshRenderer::load_vertices(Submesh const& submesh)
{
    ZoneScopedN("SubmeshRenderer::load_vertices");

    Log::debug("[SubmeshRenderer] Loading submesh vertices...");

    vao.bind();
    vbo.bind();

    std::vector<Vertex> const& vertices = submesh.get_vertices();

    Renderer::send_buffer_data(
        BufferType::ARRAY_BUFFER, static_cast<std::ptrdiff_t>(sizeof(vertices.front()) * vertices.size()),
        vertices.data(), BufferDataUsage::STATIC_DRAW
    );

    vbo.vertex_count = static_cast<uint>(vertices.size());

    constexpr uint8_t stride = sizeof(vertices.front());

    // Position
    Renderer::set_vertex_attrib(
        0, AttribDataType::FLOAT, 3, // vec3
        stride, 0
    );
    Renderer::enable_vertex_attrib_array(0);

    // Texcoords
    constexpr size_t texcoords_offset = sizeof(vertices.front().position);
    Renderer::set_vertex_attrib(
        1, AttribDataType::FLOAT, 2, // vec2
        stride, texcoords_offset
    );
    Renderer::enable_vertex_attrib_array(1);

    // Normal
    constexpr size_t normal_offset = texcoords_offset + sizeof(vertices.front().texcoords);
    Renderer::set_vertex_attrib(
        2, AttribDataType::FLOAT, 3, // vec3
        stride, normal_offset
    );
    Renderer::enable_vertex_attrib_array(2);

    // Tangent
    constexpr size_t tangent_offset = normal_offset + sizeof(vertices.front().normal);
    Renderer::set_vertex_attrib(
        3, AttribDataType::FLOAT, 3, // vec3
        stride, tangent_offset
    );
    Renderer::enable_vertex_attrib_array(3);

    vbo.unbind();
    vao.unbind();

    Log::vdebug("[SubmeshRenderer] Loaded submesh vertices ({} vertices loaded)", vertices.size());
}

void SubmeshRenderer::load_indices(Submesh const& submesh)
{
    ZoneScopedN("SubmeshRenderer::load_indices");

    Log::debug("[SubmeshRenderer] Loading submesh indices...");

    vao.bind();
    ibo.bind();

    // Mapping the indices to lines' if asked, and triangles' otherwise
    std::vector<uint> const& indices =
        (render_mode == RenderMode::LINE ? submesh.get_line_indices() : submesh.get_triangle_indices());

    Renderer::send_buffer_data(
        BufferType::ELEMENT_BUFFER, static_cast<std::ptrdiff_t>(sizeof(indices.front()) * indices.size()),
        indices.data(), BufferDataUsage::STATIC_DRAW
    );

    ibo.line_index_count = static_cast<uint>(submesh.get_line_index_count());
    ibo.triangle_index_count = static_cast<uint>(submesh.get_triangle_index_count());

    ibo.unbind();
    vao.unbind();

    Log::vdebug("[SubmeshRenderer] Loaded submesh indices ({} indices loaded)", indices.size());
}
}