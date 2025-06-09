#pragma once

#include <render/process/render_process.hpp>

namespace xen {
class FragmentShader;

class MonoPass : public RenderProcess {
public:
    MonoPass(RenderGraph& render_graph, FragmentShader&& frag_shader, std::string pass_name = {});

    [[nodiscard]] bool is_enabled() const override;

    void set_state(bool enabled) override;

    void add_parent(RenderPass& parent_pass) override;

    void add_parent(RenderProcess& parent_process) override;

    void add_child(RenderPass& child_pass) override;

    void add_child(RenderProcess& child_process) override;

    [[nodiscard]] float recover_elapsed_time() const override;

protected:
    void set_input_buffer(Texture2DPtr input_buffer, std::string const& uniform_name);

    void set_output_buffer(Texture2DPtr output_buffer, uint32_t index);

protected:
    RenderPass& pass;
};
}