#include "sobel_filter.hpp"

#include <render/render_pass.hpp>

namespace {
constexpr std::string_view sobelSource = {
#include "sobel_filter.frag.embed"
};
}

namespace xen {
SobelFilter::SobelFilter(RenderGraph& render_graph) :
    MonoPass(render_graph, FragmentShader::load_from_source(sobelSource), "Sobel filter")
{
}

void SobelFilter::resize_buffers(Vector2ui const& size)
{
    Vector2f const inv_buffer_size(1.f / static_cast<float>(size.x), 1.f / static_cast<float>(size.y));

    pass.get_program().set_attribute(inv_buffer_size, "uniInvBufferSize");
    pass.get_program().send_attributes();
}

void SobelFilter::set_input_buffer(Texture2DPtr color_buffer)
{
    resize_buffers(color_buffer->get_size());
    MonoPass::set_input_buffer(std::move(color_buffer), "uniBuffer");
}

void SobelFilter::set_output_gradient_buffer(Texture2DPtr color_buffer)
{
    MonoPass::set_output_buffer(std::move(color_buffer), 0);
}

void SobelFilter::set_output_gradient_direction_buffer(Texture2DPtr color_buffer)
{
    MonoPass::set_output_buffer(std::move(color_buffer), 1);
}
}