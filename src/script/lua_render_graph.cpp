#include <render/process/bloom.hpp>
#include <render/process/box_blur.hpp>
#include <render/process/chromatic_aberration.hpp>
#include <render/process/convolution.hpp>
#include <render/process/film_grain.hpp>
#include <render/process/gaussian_blur.hpp>
#include <render/process/pixelization.hpp>
#include <render/process/sobel_filter.hpp>
#include <render/process/vignette.hpp>

#include <render/render_graph.hpp>
#include <render/render_pass.hpp>
#include <render/process/ssr.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_render_graph_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<RenderGraph> render_graph = state.new_usertype<RenderGraph>(
            "RenderGraph", sol::constructors<RenderGraph()>(), sol::base_classes, sol::bases<Graph<RenderPass>>()
        );
        render_graph["get_node_count"] = &RenderGraph::get_node_count;
        render_graph["get_node"] = PickNonConstOverload<size_t>(&RenderGraph::get_node);
        render_graph["add_node"] = sol::overload(
            &RenderGraph::add_node<>, [](RenderGraph& g, FragmentShader& s) { g.add_node(std::move(s)); },
            [](RenderGraph& g, FragmentShader& s, std::string n) { g.add_node(std::move(s), std::move(n)); }
        );
        render_graph["remove_node"] = &RenderGraph::remove_node;

        render_graph["is_valid"] = &RenderGraph::is_valid;
        render_graph["get_geometry_pass"] = PickNonConstOverload<>(&RenderGraph::get_geometry_pass);
        render_graph["addBloom"] = &RenderGraph::add_render_process<Bloom>;
        render_graph["addBoxBlur"] = &RenderGraph::add_render_process<BoxBlur>;
        render_graph["addChromaticAberration"] = &RenderGraph::add_render_process<ChromaticAberration>;
        render_graph["addConvolutionRenderProcess"] = sol::
            overload(&RenderGraph::add_render_process<ConvolutionRenderProcess, Matrix3 const&>, &RenderGraph::add_render_process<ConvolutionRenderProcess, Matrix3 const&, std::string>);
        render_graph["addFilmGrain"] = &RenderGraph::add_render_process<FilmGrain>;
        render_graph["addGaussianBlur"] = &RenderGraph::add_render_process<GaussianBlur>;
        render_graph["addPixelization"] = &RenderGraph::add_render_process<Pixelization>;
        render_graph["addScreenSpaceReflections"] = &RenderGraph::add_render_process<ScreenSpaceReflections>;
        render_graph["addSobelFilter"] = &RenderGraph::add_render_process<SobelFilter>;
        render_graph["addVignette"] = &RenderGraph::add_render_process<Vignette>;
        render_graph["resize_viewport"] = &RenderGraph::resize_viewport;
        render_graph["update_shaders"] = &RenderGraph::update_shaders;
    }

    {
        sol::usertype<RenderPass> render_pass = state.new_usertype<RenderPass>(
            "RenderPass", sol::constructors<RenderPass()>(), sol::base_classes, sol::bases<GraphNode<RenderPass>>()
        );
        render_pass["get_parent_count"] = &RenderPass::get_parent_count;
        render_pass["get_parent"] = PickNonConstOverload<size_t>(&RenderPass::get_parent);
        render_pass["get_child_count"] = &RenderPass::get_child_count;
        render_pass["get_child"] = PickNonConstOverload<size_t>(&RenderPass::get_child);
        render_pass["is_root"] = &RenderPass::is_root;
        render_pass["is_leaf"] = &RenderPass::is_leaf;
        render_pass["is_isolated"] = &RenderPass::is_isolated;
        render_pass["add_parents"] = [](RenderPass& p, sol::variadic_args args) {
            for (auto parent : args)
                p.add_parents(parent);
        };
        render_pass["remove_parents"] = [](RenderPass& p, sol::variadic_args args) {
            for (auto parent : args)
                p.remove_parents(parent);
        };
        render_pass["add_children"] = [](RenderPass& p, sol::variadic_args args) {
            for (auto child : args)
                p.add_children(child);
        };
        render_pass["remove_children"] = [](RenderPass& p, sol::variadic_args args) {
            for (auto child : args)
                p.remove_children(child);
        };

        render_pass["is_enabled"] = &RenderPass::is_enabled;
        render_pass["get_name"] = &RenderPass::get_name;
        render_pass["get_program"] = PickNonConstOverload<>(&RenderPass::get_program);
        render_pass["get_read_texture_count"] = &RenderPass::get_read_texture_count;
        render_pass["get_read_texture"] = sol::overload(
            [](RenderPass const& p, size_t i) { return &p.get_read_texture(i); },
            [](RenderPass const& p, std::string const& n) { return &p.get_read_texture(n); }
        );
        render_pass["has_read_texture"] = &RenderPass::has_read_texture;
        render_pass["get_framebuffer"] = [](RenderPass const& p) { return &p.get_framebuffer(); };
        render_pass["recover_elapsed_time"] = &RenderPass::recover_elapsed_time;
        render_pass["set_name"] = &RenderPass::set_name;
        render_pass["set_program"] = [](RenderPass& p, RenderShaderProgram& sp) { p.set_program(std::move(sp)); };
        render_pass["enable"] =
            sol::overload([](RenderPass& p) { p.enable(); }, PickOverload<bool>(&RenderPass::enable));
        render_pass["disable"] = &RenderPass::disable;
        render_pass["is_valid"] = &RenderPass::is_valid;
        render_pass["add_read_texture"] = sol::overload(
#if !defined(USE_OPENGL_ES)
            [](RenderPass& p, Texture1DPtr t, const std::string& n) { p.add_read_texture(std::move(t), n); },
#endif
            [](RenderPass& p, Texture2DPtr t, const std::string& n) { p.add_read_texture(std::move(t), n); },
            [](RenderPass& p, Texture3DPtr t, const std::string& n) { p.add_read_texture(std::move(t), n); }
        );
        render_pass["remove_read_texture"] = &RenderPass::remove_read_texture;
        render_pass["clear_read_textures"] = &RenderPass::clear_read_textures;
        render_pass["set_write_depth_texture"] = &RenderPass::set_write_depth_texture;
        render_pass["add_write_color_texture"] = &RenderPass::add_write_color_texture;
        render_pass["remove_write_texture"] = &RenderPass::remove_write_texture;
        render_pass["clear_write_textures"] = &RenderPass::clear_write_textures;
        render_pass["resize_write_buffers"] = &RenderPass::resize_write_buffers;
        render_pass["execute"] = &RenderPass::execute;
    }

    // RenderProcess
    {
        {
            auto bloom_render = state.new_usertype<Bloom>(
                "Bloom", sol::constructors<Bloom(RenderGraph&)>(), sol::base_classes, sol::bases<RenderProcess>()
            );
            bloom_render["get_threshold_pass"] = &Bloom::get_threshold_pass;
            bloom_render["get_downscale_pass_count"] = &Bloom::get_downscale_pass_count;
            bloom_render["get_downscale_pass"] = PickNonConstOverload<size_t>(&Bloom::get_downscale_pass);
            bloom_render["get_downscale_buffer_count"] = &Bloom::get_downscale_buffer_count;
            bloom_render["get_downscale_buffer"] = [](Bloom& b, size_t i) { return &b.get_downscale_buffer(i); };
            bloom_render["get_upscale_pass_count"] = &Bloom::get_upscale_pass_count;
            bloom_render["get_upscale_pass"] = PickNonConstOverload<size_t>(&Bloom::get_upscale_pass);
            bloom_render["get_upscale_buffer_count"] = &Bloom::get_upscale_buffer_count;
            bloom_render["get_upscale_buffer"] = [](Bloom& b, size_t i) { return &b.get_upscale_buffer(i); };
            bloom_render["set_input_color_buffer"] = &Bloom::set_input_color_buffer;
            bloom_render["set_output_buffer"] = &Bloom::set_output_buffer;
            bloom_render["set_threshold_value"] = &Bloom::set_threshold_value;
        }

        {
            auto box_blur = state.new_usertype<BoxBlur>(
                "BoxBlur", sol::constructors<BoxBlur(RenderGraph&)>(), sol::base_classes,
                sol::bases<MonoPass, RenderProcess>()
            );
            box_blur["set_input_buffer"] = &BoxBlur::set_input_buffer;
            box_blur["set_output_buffer"] = &BoxBlur::set_output_buffer;
            box_blur["set_strength"] = &BoxBlur::set_strength;
        }

        {
            auto chrom_aberr = state.new_usertype<ChromaticAberration>(
                "ChromaticAberration", sol::constructors<ChromaticAberration(RenderGraph&)>(), sol::base_classes,
                sol::bases<MonoPass, RenderProcess>()
            );
            chrom_aberr["set_input_buffer"] = &ChromaticAberration::set_input_buffer;
            chrom_aberr["set_output_buffer"] = &ChromaticAberration::set_output_buffer;
            chrom_aberr["set_strength"] = &ChromaticAberration::set_strength;
            chrom_aberr["set_direction"] = &ChromaticAberration::set_direction;
            chrom_aberr["set_mask_texture"] = &ChromaticAberration::set_mask_texture;
        }

        {
            auto convolution = state.new_usertype<ConvolutionRenderProcess>(
                "ConvolutionRenderProcess",
                sol::constructors<
                    ConvolutionRenderProcess(RenderGraph&, Matrix3 const&),
                    ConvolutionRenderProcess(RenderGraph&, Matrix3 const&, std::string)>(),
                sol::base_classes, sol::bases<MonoPass, RenderProcess>()
            );
            convolution["set_input_buffer"] = &ConvolutionRenderProcess::set_input_buffer;
            convolution["set_output_buffer"] = &ConvolutionRenderProcess::set_output_buffer;
            convolution["set_kernel"] = &ConvolutionRenderProcess::set_kernel;
        }

        {
            auto gauss_blur = state.new_usertype<GaussianBlur>(
                "GaussianBlur", sol::constructors<GaussianBlur(RenderGraph&)>(), sol::base_classes,
                sol::bases<RenderProcess>()
            );
            gauss_blur["getHorizontalPass"] = [](GaussianBlur& gb) { return &gb.getHorizontalPass(); };
            gauss_blur["getVerticalPass"] = [](GaussianBlur& gb) { return &gb.getVerticalPass(); };
            gauss_blur["set_input_buffer"] = &GaussianBlur::set_input_buffer;
            gauss_blur["set_output_buffer"] = &GaussianBlur::set_output_buffer;
        }

        {
            state.new_usertype<MonoPass>(
                "MonoPass", sol::no_constructor, sol::base_classes, sol::bases<RenderProcess>()
            );
        }

        {
            sol::usertype<RenderProcess> render_process =
                state.new_usertype<RenderProcess>("RenderProcess", sol::no_constructor);
            render_process["is_enabled"] = &RenderProcess::is_enabled;
            render_process["set_state"] = &RenderProcess::set_state;
            render_process["enable"] = &RenderProcess::enable;
            render_process["disable"] = &RenderProcess::disable;
            render_process["add_parent"] = sol::overload(
                PickOverload<RenderPass&>(&RenderProcess::add_parent),
                PickOverload<RenderProcess&>(&RenderProcess::add_parent)
            );
            render_process["add_child"] = sol::overload(
                PickOverload<RenderPass&>(&RenderProcess::add_child),
                PickOverload<RenderProcess&>(&RenderProcess::add_child)
            );
            render_process["resize_buffers"] = &RenderProcess::resize_buffers;
            render_process["recover_elapsed_time"] = &RenderProcess::recover_elapsed_time;
        }

        {
            auto ssr = state.new_usertype<ScreenSpaceReflections>(
                "ScreenSpaceReflections", sol::constructors<ScreenSpaceReflections(RenderGraph&)>(), sol::base_classes,
                sol::bases<MonoPass, RenderProcess>()
            );
            ssr["set_input_depth_buffer"] = &ScreenSpaceReflections::set_input_depth_buffer;
            ssr["set_input_color_buffer"] = &ScreenSpaceReflections::set_input_color_buffer;
            ssr["set_input_blurred_color_buffer"] = &ScreenSpaceReflections::set_input_blurred_color_buffer;
            ssr["set_input_normal_buffer"] = &ScreenSpaceReflections::set_input_normal_buffer;
            ssr["set_input_specular_buffer"] = &ScreenSpaceReflections::set_input_specular_buffer;
            ssr["set_output_buffer"] = &ScreenSpaceReflections::set_output_buffer;
        }

        {
            auto sobel = state.new_usertype<SobelFilter>(
                "SobelFilter", sol::constructors<SobelFilter(RenderGraph&)>(), sol::base_classes,
                sol::bases<MonoPass, RenderProcess>()
            );
            sobel["set_input_buffer"] = &SobelFilter::set_input_buffer;
            sobel["set_output_gradient_buffer"] = &SobelFilter::set_output_gradient_buffer;
            sobel["set_output_gradient_direction_buffer"] = &SobelFilter::set_output_gradient_direction_buffer;
        }

        {
            auto vignette = state.new_usertype<Vignette>(
                "Vignette", sol::constructors<Vignette(RenderGraph&)>(), sol::base_classes,
                sol::bases<MonoPass, RenderProcess>()
            );
            vignette["set_input_buffer"] = &Vignette::set_input_buffer;
            vignette["set_output_buffer"] = &Vignette::set_output_buffer;
            vignette["set_strength"] = &Vignette::set_strength;
            vignette["set_opacity"] = &Vignette::set_opacity;
            vignette["set_color"] = &Vignette::set_color;
        }
    }
}
}