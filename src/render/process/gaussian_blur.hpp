#pragma once

#include <render/process/render_process.hpp>

namespace xen {
class GaussianBlur final : public RenderProcess {
public:
    explicit GaussianBlur(RenderGraph& render_graph);

    [[nodiscard]] bool is_enabled() const override;

    void set_state(bool enabled) override;

    void add_parent(RenderPass& parent_pass) override;

    void add_parent(RenderProcess& parent_process) override;

    void add_child(RenderPass& child_pass) override;

    void add_child(RenderProcess& child_process) override;

    void resize_buffers(Vector2ui const& size) override;

    [[nodiscard]] float recover_elapsed_time() const override;

    [[nodiscard]] RenderPass const& getHorizontalPass() const { return *horizontal_pass; }

    [[nodiscard]] RenderPass const& getVerticalPass() const { return *vertical_pass; }

    void set_input_buffer(Texture2DPtr input_buffer);

    void set_output_buffer(Texture2DPtr output_buffer);

private:
    RenderPass* horizontal_pass{};
    RenderPass* vertical_pass{};

    Texture2DPtr horizontal_buffer{};
};
}