#pragma once

#include <data/graph.hpp>
#include <render/render_pass.hpp>
#include <render/process/render_process.hpp>

namespace xen {
class Entity;
class RenderSystem;

class RenderGraph : public Graph<RenderPass> {
    friend RenderSystem;

public:
    RenderGraph() = default;
    RenderGraph(RenderGraph const&) = delete;
    RenderGraph(RenderGraph&&) = delete;

    RenderGraph& operator=(RenderGraph const&) = delete;
    RenderGraph& operator=(RenderGraph&&) = delete;

    [[nodiscard]] bool is_valid() const;

    [[nodiscard]] RenderPass const& get_geometry_pass() const { return geometry_pass; }

    [[nodiscard]] RenderPass& get_geometry_pass() { return geometry_pass; }

    /// Adds a render process to the graph.
    /// \tparam RenderProcessT Type of the process to add; must be derived from RenderProcess.
    /// \tparam Args Types of the arguments to be forwared to the render process.
    /// \param args Arguments to be forwarded to the render process.
    /// \return Reference to the newly added render process.
    template <typename RenderProcessT, typename... Args>
    RenderProcessT& add_render_process(Args&&... args);

    void resize_viewport(Vector2ui const& size);

    void update_shaders() const;

private:
    RenderPass geometry_pass{};
    std::vector<std::unique_ptr<RenderProcess>> render_processes{};
    std::unordered_set<RenderPass const*> executed_passes{};
    RenderPass const* last_executed_pass{};

private:
    /// Executes the render graph, executing all passes starting with the geometry's.
    /// \param render_system Render system executing the render graph.
    void execute(RenderSystem& render_system);

    /// Executes the geometry pass.
    /// \param render_system Render system executing the render graph.
    void execute_geometry_pass(RenderSystem& render_system) const;

    /// Executes a render pass, which in turn recursively executes its parents if they have not already been in the
    /// current frame.
    /// \param render_pass Render pass to be executed.
    void execute_pass(RenderPass const& render_pass);
};
}

#include "render_graph.inl"