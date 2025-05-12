#pragma once

#include <render/process/mono_pass.hpp>

namespace xen {
class BoxBlur final : public MonoPass {
public:
    explicit BoxBlur(RenderGraph& render_graph);

    void resize_buffers(Vector2ui const& size) override;

    void set_input_buffer(Texture2DPtr color_buffer);

    void set_output_buffer(Texture2DPtr color_buffer);

    void set_strength(uint strength) const;
};
}