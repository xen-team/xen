#pragma once

namespace xen {

class RenderGraph;
class RenderPass;
class Texture2D;
using Texture2DPtr = std::shared_ptr<Texture2D>;

/// RenderProcess class, representing a set of render passes with fixed actions; can be derived to implement post
/// effects.
class RenderProcess {
public:
    explicit RenderProcess(RenderGraph& render_graph) : render_graph{render_graph} {}

    virtual ~RenderProcess() = default;

    virtual bool is_enabled() const = 0;

    virtual void set_state(bool enabled) = 0;

    virtual void add_parent(RenderPass& parent_pass) = 0;
    virtual void add_parent(RenderProcess& parent_process) = 0;

    virtual void add_child(RenderPass& child_pass) = 0;
    virtual void add_child(RenderProcess& child_process) = 0;

    virtual void resize_buffers(Vector2ui const& size) {}

    /// Recovers the elapsed time (in milliseconds) of the process' execution.
    /// \note This action is not available with OpenGL ES and will always return 0.
    /// \return Time taken to execute the process.
    virtual float recover_elapsed_time() const { return 0.f; }

    void enable() { set_state(true); }

    void disable() { set_state(false); }

protected:
    RenderGraph& render_graph;
};
}