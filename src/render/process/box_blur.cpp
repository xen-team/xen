#include <render/process/box_blur.hpp>
#include <render/render_pass.hpp>

namespace {
constexpr std::string_view box_blur_source = {
#include "box_blur.frag.embed"
};
}

namespace xen {
BoxBlur::BoxBlur(RenderGraph& render_graph) :
    MonoPass(render_graph, FragmentShader::load_from_source(box_blur_source), "Box blur")
{
    set_strength(1);
}

void BoxBlur::resize_buffers(Vector2ui const& size)
{
    Vector2f const inv_buffer_size(1.f / static_cast<float>(size.x), 1.f / static_cast<float>(size.y));

    pass.get_program().set_attribute(inv_buffer_size, "uniInvBufferSize");
    pass.get_program().send_attributes();
}

void BoxBlur::set_input_buffer(Texture2DPtr color_buffer)
{
    resize_buffers(color_buffer->get_size());
    MonoPass::set_input_buffer(std::move(color_buffer), "uniBuffer");
}

void BoxBlur::set_output_buffer(Texture2DPtr color_buffer)
{
    MonoPass::set_output_buffer(std::move(color_buffer), 0);
}

void BoxBlur::set_strength(uint strength) const
{
    pass.get_program().set_attribute(static_cast<int>(strength), "uniKernelSize");
    pass.get_program().send_attributes();
}
}
