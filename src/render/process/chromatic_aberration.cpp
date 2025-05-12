#include "chromatic_aberration.hpp"

#include <render/render_pass.hpp>

namespace {
constexpr std::string_view chromatic_aberration_source = {
#include "chromatic_aberration.frag.embed"
};
}

namespace xen {
ChromaticAberration::ChromaticAberration(RenderGraph& render_graph) :
    MonoPass(render_graph, FragmentShader::load_from_source(chromatic_aberration_source), "Chromatic aberration")
{
    set_strength(0.f);
    set_direction(Vector2f(1.f, 0.f));
    set_mask_texture(Texture2D::create(Color::White));
}

void ChromaticAberration::resize_buffers(Vector2ui const& size)
{
    Vector2f const inv_buffer_size(1.f / static_cast<float>(size.x), 1.f / static_cast<float>(size.y));

    pass.get_program().set_attribute(inv_buffer_size, "uniInvBufferSize");
    pass.get_program().send_attributes();
}

void ChromaticAberration::set_input_buffer(Texture2DPtr color_buffer)
{
    resize_buffers(color_buffer->get_size());
    MonoPass::set_input_buffer(std::move(color_buffer), "uniBuffer");
}

void ChromaticAberration::set_output_buffer(Texture2DPtr color_buffer)
{
    MonoPass::set_output_buffer(std::move(color_buffer), 0);
}

void ChromaticAberration::set_strength(float strength) const
{
    pass.get_program().set_attribute(strength, "uniStrength");
    pass.get_program().send_attributes();
}

void ChromaticAberration::set_direction(Vector2f const& direction) const
{
    pass.get_program().set_attribute(direction, "uniDirection");
    pass.get_program().send_attributes();
}

void ChromaticAberration::set_mask_texture(Texture2DPtr mask) const
{
    pass.get_program().set_texture(std::move(mask), "uniMask");
}

}
