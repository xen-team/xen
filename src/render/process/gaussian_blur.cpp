#include <render/process/gaussian_blur.hpp>
#include <render/renderer.hpp>
#include <render/render_graph.hpp>
#include <render/texture.hpp>

namespace {
constexpr std::string_view gaussian_blur_source = {
#include "gaussian_blur.frag.embed"
};
}

namespace xen {
GaussianBlur::GaussianBlur(RenderGraph& render_graph) :
    RenderProcess(render_graph), horizontal_buffer{Texture2D::create()}
{
    // Two-pass gaussian blur based on:
    //  - https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
    //  -
    //  https://www.intel.com/content/www/us/en/developer/articles/technical/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms.html

    horizontal_pass =
        &render_graph.add_node(FragmentShader::load_from_source(gaussian_blur_source), "Gaussian blur (horizontal)");
    horizontal_pass->get_program().set_attribute(Vector2f(1.f, 0.f), "uniBlurDirection");
    horizontal_pass->get_program().send_attributes();

#if !defined(USE_OPENGL_ES)
    if (Renderer::check_version(4, 3)) {
        Renderer::set_label(
            RenderObjectType::PROGRAM, horizontal_pass->get_program().get_index(), "Gaussian blur (horizontal) program"
        );
        Renderer::set_label(
            RenderObjectType::SHADER, horizontal_pass->get_program().get_vertex_shader().get_index(),
            "Gaussian blur (horizontal) vertex shader"
        );
        Renderer::set_label(
            RenderObjectType::SHADER, horizontal_pass->get_program().get_fragment_shader().get_index(),
            "Gaussian blur (horizontal) fragment shader"
        );
    }
#endif

    vertical_pass =
        &render_graph.add_node(FragmentShader::load_from_source(gaussian_blur_source), "Gaussian blur (vertical)");
    vertical_pass->get_program().set_attribute(Vector2f(0.f, 1.f), "uniBlurDirection");
    vertical_pass->get_program().send_attributes();

    vertical_pass->add_read_texture(horizontal_buffer, "uniBuffer");

    vertical_pass->add_parents(*horizontal_pass);

#if !defined(USE_OPENGL_ES)
    if (Renderer::check_version(4, 3)) {
        Renderer::set_label(
            RenderObjectType::PROGRAM, vertical_pass->get_program().get_index(), "Gaussian blur (vertical) program"
        );
        Renderer::set_label(
            RenderObjectType::SHADER, vertical_pass->get_program().get_vertex_shader().get_index(),
            "Gaussian blur (vertical) vertex shader"
        );
        Renderer::set_label(
            RenderObjectType::SHADER, vertical_pass->get_program().get_fragment_shader().get_index(),
            "Gaussian blur (vertical) fragment shader"
        );
    }
#endif

    if (!render_graph.is_valid()) {
        throw std::runtime_error("Error: The gaussian blur process is invalid");
    }
}

bool GaussianBlur::is_enabled() const
{
    return horizontal_pass->is_enabled();
}

void GaussianBlur::set_state(bool const enabled)
{
    horizontal_pass->enable(enabled);
    vertical_pass->enable(enabled);
}

void GaussianBlur::add_parent(RenderPass& parent_pass)
{
    horizontal_pass->add_parents(parent_pass);
}

void GaussianBlur::add_parent(RenderProcess& parent_process)
{
    parent_process.add_child(*horizontal_pass);
}

void GaussianBlur::add_child(RenderPass& child_pass)
{
    vertical_pass->add_children(child_pass);
}

void GaussianBlur::add_child(RenderProcess& child_process)
{
    child_process.add_parent(*vertical_pass);
}

void GaussianBlur::resize_buffers(Vector2ui const& size)
{
    horizontal_buffer->resize(size);

    Vector2f const inv_buffer_size(1.f / static_cast<float>(size.x), 1.f / static_cast<float>(size.y));

    horizontal_pass->get_program().set_attribute(inv_buffer_size, "uniInvBufferSize");
    horizontal_pass->get_program().send_attributes();

    vertical_pass->get_program().set_attribute(inv_buffer_size, "uniInvBufferSize");
    vertical_pass->get_program().send_attributes();
}

float GaussianBlur::recover_elapsed_time() const
{
    return horizontal_pass->recover_elapsed_time() + vertical_pass->recover_elapsed_time();
}

void GaussianBlur::set_input_buffer(Texture2DPtr input_buffer)
{
    horizontal_buffer->set_colorspace(input_buffer->get_colorspace(), input_buffer->get_data_type());
    resize_buffers(input_buffer->get_size());

    horizontal_pass->clear_read_textures();
    horizontal_pass->add_read_texture(std::move(input_buffer), "uniBuffer");

    horizontal_pass->clear_write_textures();
    horizontal_pass->add_write_color_texture(horizontal_buffer, 0);

#if !defined(USE_OPENGL_ES)
    if (Renderer::check_version(4, 3)) {
        Renderer::set_label(
            RenderObjectType::FRAMEBUFFER, horizontal_pass->get_framebuffer().get_index(),
            "Gaussian blur (horizontal) framebuffer"
        );
        Renderer::set_label(
            RenderObjectType::TEXTURE, horizontal_buffer->get_index(), "Gaussian blurred (horizontal) buffer"
        );
    }
#endif

    if (!render_graph.is_valid()) {
        throw std::runtime_error("Error: The gaussian blur process is invalid");
    }
}

void GaussianBlur::set_output_buffer(Texture2DPtr output_buffer)
{
    vertical_pass->add_write_color_texture(std::move(output_buffer), 0);

#if !defined(USE_OPENGL_ES)
    if (Renderer::check_version(4, 3))
        Renderer::set_label(
            RenderObjectType::FRAMEBUFFER, vertical_pass->get_framebuffer().get_index(),
            "Gaussian blur (vertical) framebuffer"
        );
#endif
}
}
