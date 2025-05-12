#include <render/render_graph.hpp>
#include <render/process/ssr.hpp>
#include <render/texture.hpp>

namespace {
constexpr std::string_view ssr_source = {
#include "ssr.frag.embed"
};

constexpr std::string_view depth_uniform_name = "uniSceneBuffers.depth";
constexpr std::string_view color_uniform_name = "uniSceneBuffers.color";
constexpr std::string_view blurred_color_uniform_name = "uniSceneBuffers.blurredColor";
constexpr std::string_view normal_uniform_name = "uniSceneBuffers.normal";
constexpr std::string_view specular_uniform_name = "uniSceneBuffers.specular";
}

namespace xen {
ScreenSpaceReflections::ScreenSpaceReflections(RenderGraph& render_graph) :
    MonoPass(render_graph, FragmentShader::load_from_source(ssr_source), "Screen space reflections (SSR)")
{
}

void ScreenSpaceReflections::set_input_depth_buffer(Texture2DPtr depth_buffer)
{
    Log::rt_assert(
        depth_buffer->get_colorspace() == TextureColorspace::DEPTH,
        "Error: The SSR's input depth buffer has an invalid colorspace."
    );
    set_input_buffer(std::move(depth_buffer), std::string(depth_uniform_name));
}

void ScreenSpaceReflections::set_input_color_buffer(Texture2DPtr color_buffer)
{
    if (!pass.has_read_texture(std::string(blurred_color_uniform_name))) {
        set_input_blurred_color_buffer(color_buffer);
    }

    set_input_buffer(std::move(color_buffer), std::string(color_uniform_name));
}

void ScreenSpaceReflections::set_input_blurred_color_buffer(Texture2DPtr blurred_color_buffer)
{
    set_input_buffer(std::move(blurred_color_buffer), std::string(blurred_color_uniform_name));
}

void ScreenSpaceReflections::set_input_normal_buffer(Texture2DPtr normal_buffer)
{
    Log::rt_assert(
        normal_buffer->get_colorspace() == TextureColorspace::RGB,
        "Error: The SSR's input normal buffer has an invalid colorspace."
    );
    set_input_buffer(std::move(normal_buffer), std::string(normal_uniform_name));
}

void ScreenSpaceReflections::set_input_specular_buffer(Texture2DPtr specular_buffer)
{
    Log::rt_assert(
        specular_buffer->get_colorspace() == TextureColorspace::RGBA,
        "Error: The SSR's input specular buffer has an invalid colorspace."
    );
    set_input_buffer(std::move(specular_buffer), std::string(specular_uniform_name));
}

void ScreenSpaceReflections::set_output_buffer(Texture2DPtr output_buffer)
{
    MonoPass::set_output_buffer(std::move(output_buffer), 0);
}
}