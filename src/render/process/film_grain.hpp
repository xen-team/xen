#pragma once

#include <render/process/mono_pass.hpp>

namespace xen {
class FilmGrain final : public MonoPass {
public:
    explicit FilmGrain(RenderGraph& render_graph);

    void set_input_buffer(Texture2DPtr color_buffer);

    void set_output_buffer(Texture2DPtr color_buffer);

    void set_strength(float strength) const;
};
}