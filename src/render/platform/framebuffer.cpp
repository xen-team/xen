#include "framebuffer.hpp"

#include <data/mesh.hpp>
#include <render/graphic_objects.hpp>
#include <render/renderer.hpp>
#include <render/shader/shader_program.hpp>
#include <render/texture.hpp>

#include <tracy/Tracy.hpp>
#include <GL/glew.h> // Needed by TracyOpenGL.hpp
#include <tracy/TracyOpenGL.hpp>

namespace xen {
namespace {
inline void draw_display_surface()
{
    ZoneScopedN("[Framebuffer]::draw_display_surface");

    // Creating a triangle large enough to cover the whole render frame:
    //
    //   3 | \                                3 | \
  //     |    \                               |  \
  //   2 |       \                          2 |    \
  //     |          \                         |     \
  //   1 ------------- \                    1 -------\
  //     |           |    \                   |     | \
  //   0 |           |       \              0 |     |   \
  //     |           |          \             |     |    \
  //  -1 -------------------------         -1 -------------
    //    -1     0     1     2     3           -1  0  1  2  3

    static std::pair<VertexArray, VertexBuffer> const vertex_objects = []() {
        VertexArray vao;
        VertexBuffer vbo;

        vao.bind();
        vbo.bind();

        static constexpr std::array<Vector2f, 6> vertices = {Vector2f(-1.f, -1.f), Vector2f(0.f, 0.f),
                                                             Vector2f(3.f, -1.f),  Vector2f(2.f, 0.f),
                                                             Vector2f(-1.f, 3.f),  Vector2f(0.f, 2.f)};
        Renderer::send_buffer_data(
            BufferType::ARRAY_BUFFER, static_cast<std::ptrdiff_t>(sizeof(vertices)), vertices.data(),
            BufferDataUsage::STATIC_DRAW
        );

        Renderer::set_vertex_attrib(0, AttribDataType::FLOAT, 2, sizeof(Vector2f) * 2, 0);                // Position
        Renderer::set_vertex_attrib(1, AttribDataType::FLOAT, 2, sizeof(Vector2f) * 2, sizeof(Vector2f)); // Texcoords
        Renderer::enable_vertex_attrib_array(0);
        Renderer::enable_vertex_attrib_array(1);

        vbo.unbind();
        vao.unbind();

        return std::make_pair(std::move(vao), std::move(vbo));
    }();

    vertex_objects.first.bind();
    Renderer::draw_arrays(PrimitiveType::TRIANGLES, 3);
}
}

Framebuffer::Framebuffer()
{
    ZoneScopedN("Framebuffer::Framebuffer");

    Log::debug("[Framebuffer] Creating...");
    Renderer::generate_framebuffer(index);
    Log::debug("[Framebuffer] Created (ID: " + std::to_string(index) + ")");
}

VertexShader Framebuffer::recover_vertex_shader()
{
    static constexpr std::string_view vert_source = R"(
    layout(location = 0) in vec2 vertPosition;
    layout(location = 1) in vec2 vertTexcoords;

    out vec2 fragTexcoords;

    void main() {
      fragTexcoords = vertTexcoords;
      gl_Position   = vec4(vertPosition, 0.0, 1.0);
    }
  )";

    return VertexShader::load_from_source(vert_source);
}

void Framebuffer::set_depth_buffer(Texture2DPtr texture)
{
    if (texture->get_colorspace() != TextureColorspace::DEPTH) {
        throw std::invalid_argument("Error: Invalid depth buffer");
    }

    depth_buffer = std::move(texture);

    map_buffers();
}

void Framebuffer::add_color_buffer(Texture2DPtr texture, uint32_t index)
{
    if (texture->get_colorspace() == TextureColorspace::DEPTH ||
        texture->get_colorspace() == TextureColorspace::INVALID) {
        throw std::invalid_argument("Error: Invalid color buffer");
    }

    auto const buffer_it =
        std::find_if(color_buffers.cbegin(), color_buffers.cend(), [&texture, index](auto const& color_buffer) {
            return (texture == color_buffer.first && index == color_buffer.second);
        });

    // Adding the color buffer only if it doesn't exist yet
    if (buffer_it == color_buffers.cend()) {
        color_buffers.emplace_back(std::move(texture), index);
    }

    map_buffers();
}

void Framebuffer::remove_texture_buffer(Texture2DPtr const& texture)
{
    if (texture == depth_buffer) {
        depth_buffer.reset();
    }
    else {
        auto const buffer_it =
            std::remove_if(color_buffers.begin(), color_buffers.end(), [&texture](auto const& buffer) {
                return (texture == buffer.first);
            });
        color_buffers.erase(buffer_it, color_buffers.end());
    }

    map_buffers();
}

void Framebuffer::clear_texture_buffers()
{
    clear_depth_buffer();
    clear_color_buffers();
}

void Framebuffer::resize_buffers(Vector2ui const& size)
{
    ZoneScopedN("Framebuffer::resize_buffers");

    if (depth_buffer) {
        depth_buffer->resize(size);
    }

    for (auto const& [color_buffer, _] : color_buffers) {
        color_buffer->resize(size);
    }
}

void Framebuffer::map_buffers() const
{
    ZoneScopedN("Framebuffer::map_buffers");

    Log::debug("[Framebuffer] Mapping buffers (ID: " + std::to_string(index) + ")...");

    Renderer::bind_framebuffer(index);

    if (depth_buffer) {
        Log::debug("[Framebuffer] Mapping depth buffer...");
        Renderer::set_framebuffer_texture_2d(
            FramebufferAttachment::DEPTH, depth_buffer->get_index(), 0, TextureType::TEXTURE_2D
        );
    }

    if (!color_buffers.empty()) {
        std::vector<DrawBuffer> draw_buffers(color_buffers.size(), DrawBuffer::NONE);

        for (auto const& [color_buffer, buffer_index] : color_buffers) {
            Log::debug("[Framebuffer] Mapping color buffer " + std::to_string(buffer_index) + "...");

            size_t const color_attachment = static_cast<uint32_t>(DrawBuffer::COLOR_ATTACHMENT0) + buffer_index;

            Renderer::set_framebuffer_texture_2d(
                static_cast<FramebufferAttachment>(color_attachment), color_buffer->get_index(), 0,
                TextureType::TEXTURE_2D
            );

            if (buffer_index >= draw_buffers.size()) {
                draw_buffers.resize(buffer_index + 1, DrawBuffer::NONE);
            }
            draw_buffers[buffer_index] = static_cast<DrawBuffer>(color_attachment);
        }

        Renderer::set_draw_buffers(static_cast<uint32_t>(draw_buffers.size()), draw_buffers.data());
    }

    unbind();

    Log::debug("[Framebuffer] Mapped buffers");
}

void Framebuffer::bind() const
{
    Renderer::bind_framebuffer(index);
    Renderer::clear(MaskType::COLOR | MaskType::DEPTH | MaskType::STENCIL);
}

void Framebuffer::unbind() const
{
    Renderer::unbind_framebuffer();
}

void Framebuffer::display() const
{
    ZoneScopedN("Framebuffer::display");
    TracyGpuZone("Framebuffer::display");

    Renderer::clear(MaskType::COLOR);
    draw_display_surface();
}

Framebuffer::~Framebuffer()
{
    ZoneScopedN("Framebuffer::~Framebuffer");

    if (!index.is_valid())
        return;

    Log::debug("[Framebuffer] Destroying (ID: " + std::to_string(index) + ")...");
    Renderer::deleteFramebuffer(index);
    Log::debug("[Framebuffer] Destroyed");
}

}
