#include <render/process/mono_pass.hpp>
#include <render/renderer.hpp>
#include <render/render_graph.hpp>
#include <render/shader/shader.hpp>
#include <utils/filepath.hpp>

namespace xen {
MonoPass::MonoPass(RenderGraph& render_graph, FragmentShader&& frag_shader, std::string pass_name) :
    RenderProcess(render_graph), pass{render_graph.add_node(std::move(frag_shader), std::move(pass_name))}
{
#if !defined(USE_OPENGL_ES)
    if (Renderer::check_version(4, 3)) {
        Renderer::set_label(RenderObjectType::PROGRAM, pass.get_program().get_index(), pass.get_name() + " program");
        Renderer::set_label(
            RenderObjectType::SHADER, pass.get_program().get_vertex_shader().get_index(),
            pass.get_name() + " vertex shader"
        );
        Renderer::set_label(
            RenderObjectType::SHADER, pass.get_program().get_fragment_shader().get_index(),
            pass.get_name() + " fragment shader"
        );
    }
#endif
}

bool MonoPass::is_enabled() const
{
    return pass.is_enabled();
}

void MonoPass::set_state(bool const enabled)
{
    pass.enable(enabled);
}

void MonoPass::add_parent(RenderPass& parent_pass)
{
    pass.add_parents(parent_pass);
}

void MonoPass::add_parent(RenderProcess& parent_process)
{
    parent_process.add_child(pass);
}

void MonoPass::add_child(RenderPass& child_pass)
{
    pass.add_children(child_pass);
}

void MonoPass::add_child(RenderProcess& child_process)
{
    child_process.add_parent(pass);
}

float MonoPass::recover_elapsed_time() const
{
    return pass.recover_elapsed_time();
}

void MonoPass::set_input_buffer(Texture2DPtr input_buffer, std::string const& uniform_name)
{
    pass.add_read_texture(std::move(input_buffer), uniform_name);
}

void MonoPass::set_output_buffer(Texture2DPtr output_buffer, uint32_t index)
{
    pass.add_write_color_texture(std::move(output_buffer), index);

#if !defined(USE_OPENGL_ES)
    // This label could be added in the constructor. However, although it does work, adding a label to an empty
    // framebuffer
    //  (with no write texture) produces an OpenGL error, which is avoided here
    if (Renderer::check_version(4, 3))
        Renderer::set_label(
            RenderObjectType::FRAMEBUFFER, pass.get_framebuffer().get_index(), pass.get_name() + " framebuffer"
        );
#endif
}

}
