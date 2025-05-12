#include "canny_filter.hpp"

#include <render/render_pass.hpp>

namespace {
constexpr std::string_view canny_source = {
#include "canny_filter.frag.embed"
};
}

namespace xen {
CannyFilter::CannyFilter(RenderGraph& render_graph) :
    MonoPass(render_graph, FragmentShader::load_from_source(canny_source), "Canny filter")
{
    set_lower_bound(0.1f);
    set_upper_bound(0.3f);
}

void CannyFilter::resize_buffers(Vector2ui const& size)
{
    Vector2f const inv_buffer_size(1.f / static_cast<float>(size.x), 1.f / static_cast<float>(size.y));

    pass.get_program().set_attribute(inv_buffer_size, "uniInvBufferSize");
    pass.get_program().send_attributes();
}

void CannyFilter::set_input_gradient_buffer(Texture2DPtr gradient_buffer)
{
    Log::rt_assert(gradient_buffer != nullptr, "Error: The input gradient buffer is invalid.");

    resize_buffers(gradient_buffer->get_size());
    MonoPass::set_input_buffer(std::move(gradient_buffer), "uniGradients");
}

void CannyFilter::set_input_gradient_direction_buffer(Texture2DPtr grad_dir_buffer)
{
    MonoPass::set_input_buffer(std::move(grad_dir_buffer), "uniGradDirs");
}

void CannyFilter::set_output_buffer(Texture2DPtr binary_buffer)
{
    MonoPass::set_output_buffer(std::move(binary_buffer), 0);
}

void CannyFilter::set_lower_bound(float const lower_bound) const
{
    pass.get_program().set_attribute(lower_bound, "uniLowerBound");
    pass.get_program().send_attributes();
}

void CannyFilter::set_upper_bound(float const upper_bound) const
{
    pass.get_program().set_attribute(upper_bound, "uniUpperBound");
    pass.get_program().send_attributes();
}

}
