#pragma once

#include <render/process/mono_pass.hpp>

namespace xen {
class ConvolutionRenderProcess : public MonoPass {
public:
    ConvolutionRenderProcess(RenderGraph& render_graph, Matrix3 const& kernel, std::string pass_name = "Convolution");

    void resize_buffers(Vector2ui const& size) override;

    void set_input_buffer(Texture2DPtr color_buffer);

    void set_output_buffer(Texture2DPtr color_buffer);

    void set_kernel(Matrix3 const& kernel) const;
};
}