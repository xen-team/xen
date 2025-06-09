#include "convolution.hpp"

#include <render/render_pass.hpp>

namespace {
constexpr std::string_view convolution_source = {
#include "convolution.frag.embed"
};
}

namespace xen {
ConvolutionRenderProcess::ConvolutionRenderProcess(
    RenderGraph& render_graph, Matrix3 const& kernel, std::string pass_name
) : MonoPass(render_graph, FragmentShader::load_from_source(convolution_source), std::move(pass_name))
{
    set_kernel(kernel);
}

void ConvolutionRenderProcess::resize_buffers(Vector2ui const& size)
{
    Vector2f const inv_buffer_size(1.f / static_cast<float>(size.x), 1.f / static_cast<float>(size.y));

    pass.get_program().set_attribute(inv_buffer_size, "uniInvBufferSize");
    pass.get_program().send_attributes();
}

void ConvolutionRenderProcess::set_input_buffer(Texture2DPtr color_buffer)
{
    resize_buffers(color_buffer->get_size());
    MonoPass::set_input_buffer(std::move(color_buffer), "uniBuffer");
}

void ConvolutionRenderProcess::set_output_buffer(Texture2DPtr color_buffer)
{
    MonoPass::set_output_buffer(std::move(color_buffer), 0);
}

void ConvolutionRenderProcess::set_kernel(Matrix3 const& kernel) const
{
    pass.get_program().set_attribute(kernel, "uniKernel");
    // pass.get_program().set_attribute(std::vector<float>(kernel.data(), kernel.data() + 9),
    // "uniKernel");
    pass.get_program().send_attributes();
}
}
