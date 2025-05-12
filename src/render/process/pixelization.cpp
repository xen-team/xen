#include "pixelization.hpp"

#include <render/render_pass.hpp>

namespace {
constexpr std::string_view pixelizationSource = {
#include "pixelization.frag.embed"
};
}

namespace xen {
Pixelization::Pixelization(RenderGraph& render_graph) :
    MonoPass(render_graph, FragmentShader::load_from_source(pixelizationSource), "Pixelization")
{
    set_strength(0.f);
}

void Pixelization::resize_buffers(Vector2ui const& size)
{
    Vector2f const buffer_size(size);

    pass.get_program().set_attribute(buffer_size, "uniBufferSize");
    pass.get_program().send_attributes();
}

void Pixelization::set_input_buffer(Texture2DPtr color_buffer)
{
    resize_buffers(color_buffer->get_size());
    MonoPass::set_input_buffer(std::move(color_buffer), "uniBuffer");
}

void Pixelization::set_output_buffer(Texture2DPtr color_buffer)
{
    MonoPass::set_output_buffer(std::move(color_buffer), 0);
}

void Pixelization::set_strength(float const strength) const
{
    pass.get_program().set_attribute(strength, "uniStrength");
    pass.get_program().send_attributes();
}
}
