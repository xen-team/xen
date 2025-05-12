#pragma once

#include <render/process/mono_pass.hpp>

namespace xen {

class Pixelization final : public MonoPass {
public:
    explicit Pixelization(RenderGraph& render_graph);

    void resize_buffers(Vector2ui const& size) override;

    void set_input_buffer(Texture2DPtr color_buffer);

    void set_output_buffer(Texture2DPtr color_buffer);

    /// Sets the pixelization strength.
    /// \param strength Strength value between 0 (no pixelization) and 1 (the whole screen is taken by one single
    /// pixel).
    void set_strength(float strength) const;
};
}