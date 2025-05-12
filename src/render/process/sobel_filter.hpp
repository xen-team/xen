#pragma once

#include <render/process/mono_pass.hpp>

namespace xen {
/// [Sobel filter/operator](https://en.wikipedia.org/wiki/Sobel_operator) render process.
class SobelFilter final : public MonoPass {
public:
    explicit SobelFilter(RenderGraph& render_graph);

    void resize_buffers(Vector2ui const& size) override;

    void set_input_buffer(Texture2DPtr color_buffer);

    /// Sets the output buffer which will contain the gradient values.
    /// \param gradient_buffer Gradient buffer.
    void set_output_gradient_buffer(Texture2DPtr gradient_buffer);

    /// Sets the output buffer which will contain the gradient direction values.
    ///
    ///           /--0.75--\
  ///         /            \
  ///       /                \
  ///     0.5                0/1
    ///       \                /
    ///         \            /
    ///           \--0.25--/
    ///
    /// \note The direction values are just like those of [atan2](https://en.wikipedia.org/wiki/Atan2) (see image
    /// below), but remapped between [0; 1].
    /// \imageSize{https://upload.cppreference.com/mwiki/images/9/91/math-atan2.png, height: 20%; width: 20%;}
    /// \image html https://upload.cppreference.com/mwiki/images/9/91/math-atan2.png
    /// \param grad_dir_buffer Gradient direction buffer.
    void set_output_gradient_direction_buffer(Texture2DPtr grad_dir_buffer);
};
}