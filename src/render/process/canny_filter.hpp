#pragma once

#include <render/process/mono_pass.hpp>

namespace xen {

/// [Canny filter/edge detector](https://en.wikipedia.org/wiki/Canny_edge_detector) render process.
/// Detects the edges within an image given its pixels' gradient information.
class CannyFilter final : public MonoPass {
public:
    explicit CannyFilter(RenderGraph& render_graph);

    void resize_buffers(Vector2ui const& size) override;

    /// Sets the given gradient buffer as input.
    /// \param gradient_buffer Buffer containing the gradient values. Obtained from another filter such as Sobel.
    /// \see SobelFilter
    void set_input_gradient_buffer(Texture2DPtr gradient_buffer);

    /// Sets the given gradient direction buffer as input.
    /// \param grad_dir_buffer Buffer containing the gradient direction values. Obtained from another filter such as
    /// Sobel.
    /// \see SobelFilter
    void set_input_gradient_direction_buffer(Texture2DPtr grad_dir_buffer);

    void set_output_buffer(Texture2DPtr binary_buffer);

    void set_lower_bound(float lower_bound) const;

    void set_upper_bound(float upper_bound) const;
};

}