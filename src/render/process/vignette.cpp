#include <render/process/vignette.hpp>
#include <render/render_pass.hpp>

namespace {
constexpr std::string_view vignetteSource = {
#include "vignette.frag.embed"
};
}

namespace xen {
Vignette::Vignette(RenderGraph& render_graph) :
    MonoPass(render_graph, FragmentShader::load_from_source(vignetteSource), "Vignette")
{
    set_strength(0.25f);
    set_opacity(1.f);
    set_color(Color::Black);
}

void Vignette::resize_buffers(Vector2ui const& size)
{
    float const frame_ratio = static_cast<float>(size.x) / static_cast<float>(size.y);
    pass.get_program().set_attribute(frame_ratio, "uniFrameRatio");
    pass.get_program().send_attributes();
}

void Vignette::set_input_buffer(Texture2DPtr color_buffer)
{
    resize_buffers(color_buffer->get_size());
    MonoPass::set_input_buffer(std::move(color_buffer), "uniBuffer");
}

void Vignette::set_output_buffer(Texture2DPtr color_buffer)
{
    MonoPass::set_output_buffer(std::move(color_buffer), 0);
}

void Vignette::set_strength(float strength) const
{
    pass.get_program().set_attribute(strength, "uniStrength");
    pass.get_program().send_attributes();
}

void Vignette::set_opacity(float opacity) const
{
    pass.get_program().set_attribute(opacity, "uniOpacity");
    pass.get_program().send_attributes();
}

void Vignette::set_color(Color const& color) const
{
    pass.get_program().set_attribute(color, "uniColor");
    pass.get_program().send_attributes();
}
}