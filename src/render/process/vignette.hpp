#pragma once

#include <render/process/mono_pass.hpp>

namespace xen {
class Vignette final : public MonoPass {
public:
    explicit Vignette(RenderGraph& render_graph);

    void resize_buffers(Vector2ui const& size) override;

    void set_input_buffer(Texture2DPtr color_buffer);

    void set_output_buffer(Texture2DPtr color_buffer);

    void set_strength(float strength) const;

    void set_opacity(float opacity) const;

    void set_color(Color const& color) const;
};
}