#include <render/process/film_grain.hpp>
#include <render/render_pass.hpp>

namespace {
constexpr std::string_view film_grain_source = {
#include "film_grain.frag.embed"
};
}

namespace xen {
FilmGrain::FilmGrain(RenderGraph& render_graph) :
    MonoPass(render_graph, FragmentShader::load_from_source(film_grain_source), "Film grain")
{
    set_strength(0.05f);
}

void FilmGrain::set_input_buffer(Texture2DPtr color_buffer)
{
    MonoPass::set_input_buffer(std::move(color_buffer), "uniBuffer");
}

void FilmGrain::set_output_buffer(Texture2DPtr color_buffer)
{
    MonoPass::set_output_buffer(std::move(color_buffer), 0);
}

void FilmGrain::set_strength(float strength) const
{
    pass.get_program().set_attribute(strength, "uniStrength");
    pass.get_program().send_attributes();
}
}
