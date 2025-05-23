#pragma once

#include <render/process/render_process.hpp>

namespace xen {
class Bloom final : public RenderProcess {
public:
    explicit Bloom(RenderGraph& render_graph);

    [[nodiscard]] bool is_enabled() const override;

    void set_state(bool enabled) override;

    void add_parent(RenderPass& parent_pass) override;

    void add_parent(RenderProcess& parent_process) override;

    void add_child(RenderPass& child_pass) override;

    void add_child(RenderProcess& child_process) override;

    void resize_buffers(Vector2ui const& size) override;

    [[nodiscard]] float recover_elapsed_time() const override;

    [[nodiscard]] RenderPass& get_threshold_pass() { return *threshold_pass; }

    [[nodiscard]] size_t get_downscale_pass_count() const { return downscale_passes.size(); }

    [[nodiscard]] RenderPass const& get_downscale_pass(size_t pass_index) const
    {
        return *downscale_passes[pass_index];
    }

    [[nodiscard]] RenderPass& get_downscale_pass(size_t pass_index) { return *downscale_passes[pass_index]; }

    [[nodiscard]] size_t get_downscale_buffer_count() const { return downscale_buffers.size(); }

    [[nodiscard]] Texture2D const& get_downscale_buffer(size_t buffer_index) const
    {
        return *downscale_buffers[buffer_index].lock();
    }

    [[nodiscard]] size_t get_upscale_pass_count() const { return upscale_passes.size(); }

    [[nodiscard]] RenderPass const& get_upscale_pass(size_t pass_index) const { return *upscale_passes[pass_index]; }

    [[nodiscard]] RenderPass& get_upscale_pass(size_t pass_index) { return *upscale_passes[pass_index]; }

    [[nodiscard]] size_t get_upscale_buffer_count() const { return upscale_buffers.size(); }

    [[nodiscard]] Texture2D const& get_upscale_buffer(size_t buffer_index) const
    {
        return *upscale_buffers[buffer_index].lock();
    }

    void set_input_color_buffer(Texture2DPtr color_buffer);

    void set_output_buffer(Texture2DPtr output_buffer);

    void set_threshold_value(float threshold) const;

private:
    RenderPass* threshold_pass = nullptr;

    std::vector<RenderPass*> downscale_passes;
    std::vector<std::weak_ptr<Texture2D>> downscale_buffers;

    std::vector<RenderPass*> upscale_passes;
    std::vector<std::weak_ptr<Texture2D>> upscale_buffers;

    RenderPass* final_pass = nullptr;
};
}