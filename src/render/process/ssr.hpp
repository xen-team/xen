#pragma once

#include <render/process/mono_pass.hpp>

namespace xen {
class ScreenSpaceReflections final : public MonoPass {
public:
    explicit ScreenSpaceReflections(RenderGraph& render_graph);

    void set_input_depth_buffer(Texture2DPtr depth_buffer);

    void set_input_color_buffer(Texture2DPtr color_buffer);

    void set_input_blurred_color_buffer(Texture2DPtr blurred_color_buffer);

    void set_input_normal_buffer(Texture2DPtr normal_buffer);

    void set_input_specular_buffer(Texture2DPtr specular_buffer);

    void set_output_buffer(Texture2DPtr output_buffer);
};
}