#pragma once

#include <render/process/mono_pass.hpp>

namespace xen {
class ChromaticAberration final : public MonoPass {
public:
    explicit ChromaticAberration(RenderGraph& render_graph);

    void resize_buffers(Vector2ui const& size) override;

    void set_input_buffer(Texture2DPtr color_buffer);

    void set_output_buffer(Texture2DPtr color_buffer);

    void set_strength(float strength) const;

    void set_direction(Vector2f const& direction) const;

    void set_mask_texture(Texture2DPtr mask) const;
};
}