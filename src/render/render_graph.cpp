#include "render_graph.hpp"

// #include <math/transform/transform.hpp>
#include <render/camera.hpp>
#include <render/mesh_renderer.hpp>
#include <render/render_system.hpp>

#include <tracy/Tracy.hpp>
#include <GL/glew.h> // Needed by TracyOpenGL.hpp
#include <tracy/TracyOpenGL.hpp>

namespace xen {
bool RenderGraph::is_valid() const
{
    return std::all_of(nodes.cbegin(), nodes.cend(), [](std::unique_ptr<RenderPass> const& render_pass) {
        return render_pass->is_valid();
    });
}

void RenderGraph::resize_viewport(Vector2ui const& size)
{
    ZoneScopedN("RenderGraph::resize_viewport");

    geometry_pass.resize_write_buffers(size);

    for (std::unique_ptr<RenderPass> const& render_pass : nodes) {
        render_pass->resize_write_buffers(size
        ); // TODO: resizing all write buffers will only work if they have all been created with equal dimensions
    }

    for (std::unique_ptr<RenderProcess> const& render_process : render_processes) {
        render_process->resize_buffers(size);
    }
}

void RenderGraph::update_shaders() const
{
    ZoneScopedN("RenderGraph::update_shaders");

    for (std::unique_ptr<RenderPass> const& render_pass : nodes) {
        render_pass->get_program().update_shaders();
    }
}

void RenderGraph::execute(RenderSystem& render_system)
{
    ZoneScopedN("RenderGraph::execute");

    {
        ZoneScopedN("Renderer::clear");
        Renderer::clear(MaskType::COLOR | MaskType::DEPTH | MaskType::STENCIL);
    }

    execute_geometry_pass(render_system);
    last_executed_pass = &geometry_pass;

    executed_passes.reserve(nodes.size() + 1);
    executed_passes.emplace(&geometry_pass);

    for (std::unique_ptr<RenderPass> const& render_pass : nodes) {
        execute_pass(*render_pass);
    }

    executed_passes.clear();
}

void RenderGraph::execute_geometry_pass(RenderSystem& render_system) const
{
    ZoneScopedN("RenderGraph::execute_geometry_pass");
    TracyGpuZone("Geometry pass")

#if !defined(USE_OPENGL_ES)
        geometry_pass.timer.start();

#if defined(XEN_CONFIG_DEBUG)
    if (Renderer::check_version(4, 3)) {
        Renderer::push_debug_group("Geometry pass");
    }
#endif
#endif

    const Framebuffer& geometry_framebuffer = geometry_pass.write_framebuffer;

    if (!geometry_framebuffer.empty()) {
        geometry_framebuffer.bind();
    }

    if (render_system.has_cubemap()) {
        render_system.get_cubemap().draw();
    }

    render_system.model_ubo.bind();

    for (Entity const* entity : render_system.entities) {
        if (!entity->is_enabled() || !entity->has_component<MeshRenderer>() || !entity->has_component<Transform>()) {
            continue;
        }

        auto const& mesh_renderer = entity->get_component<MeshRenderer>();

        if (!mesh_renderer.is_enabled()) {
            continue;
        }

        render_system.model_ubo.send_data(entity->get_component<Transform>().compute_transform(), 0);
        mesh_renderer.draw();
    }

    geometry_framebuffer.unbind();

#if !defined(USE_OPENGL_ES)
    geometry_pass.timer.stop();

#if defined(XEN_CONFIG_DEBUG)
    if (Renderer::check_version(4, 3)) {
        Renderer::pop_debug_group();
    }
#endif
#endif
}

void RenderGraph::execute_pass(RenderPass const& render_pass)
{
    if (executed_passes.find(&render_pass) != executed_passes.cend()) {
        return;
    }

    for (RenderPass const* parent_pass : render_pass.parents) {
        execute_pass(*parent_pass);
    }

    render_pass.execute();
    last_executed_pass = &render_pass;

    executed_passes.emplace(&render_pass);
}
}