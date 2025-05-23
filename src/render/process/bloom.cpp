#include <render/process/bloom.hpp>
#include <render/renderer.hpp>
#include <render/render_graph.hpp>
#include <render/texture.hpp>

namespace {
constexpr int pass_count = 5;

constexpr std::string_view threshold_source = R"(
  in vec2 fragTexcoords;

  uniform sampler2D uniColorBuffer;
  uniform float uniThreshold;

  layout(location = 0) out vec4 fragColor;

  void main() {
    vec3 color = texture(uniColorBuffer, fragTexcoords).rgb;

    // Thresholding pixels according to their luminance: https://en.wikipedia.org/wiki/Luma_(video)#Use_of_relative_luminance
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    fragColor        = vec4(color * float(brightness >= uniThreshold), 1.0);
  }
)";

constexpr std::string_view downscale_source = R"(
  in vec2 fragTexcoords;

  uniform sampler2D uniPrevDownscaledBuffer;
  uniform vec2 uniInvBufferSize;

  layout(location = 0) out vec4 fragColor;

  const vec2 kernelOffsets[13] = vec2[](
    vec2(-1.0,  1.0), vec2(1.0,  1.0),
    vec2(-1.0, -1.0), vec2(1.0, -1.0),

    vec2(-2.0,  2.0), vec2(0.0,  2.0), vec2(2.0,  2.0),
    vec2(-2.0,  0.0), vec2(0.0,  0.0), vec2(2.0,  0.0),
    vec2(-2.0, -2.0), vec2(0.0, -2.0), vec2(2.0, -2.0)
  );

  const float kernelWeights[13] = float[](
    // 4 inner samples: (1 / 4) * 0.5
    0.125, 0.125,
    0.125, 0.125,

    // 1 middle & 8 outer samples: (1 / 9) * 0.5
    0.0555555, 0.0555555, 0.0555555,
    0.0555555, 0.0555555, 0.0555555,
    0.0555555, 0.0555555, 0.0555555
  );

  void main() {
    vec3 color = vec3(0.0);

    for (int i = 0; i < 13; ++i) {
      vec2 normalizedTexcoords = (gl_FragCoord.xy + kernelOffsets[i]) * uniInvBufferSize;
      color += texture(uniPrevDownscaledBuffer, normalizedTexcoords).rgb * kernelWeights[i];
    }

    fragColor = vec4(color, 1.0);
  }
)";

constexpr std::string_view upscale_source = R"(
  in vec2 fragTexcoords;

  uniform sampler2D uniDownscaledBuffer;
  uniform sampler2D uniPrevUpscaledBuffer;
  uniform vec2 uniInvBufferSize;

  layout(location = 0) out vec4 fragColor;

  const vec2 kernelOffsets[9] = vec2[](
      vec2(-1.0,  1.0), vec2(0.0,  1.0), vec2(1.0,  1.0),
      vec2(-1.0,  0.0), vec2(0.0,  0.0), vec2(1.0,  0.0),
      vec2(-1.0, -1.0), vec2(0.0, -1.0), vec2(1.0, -1.0)
  );

  const float kernelWeights[9] = float[](
      0.0625, 0.125, 0.0625,
      0.125,  0.25,  0.125,
      0.0625, 0.125, 0.0625
  );

  void main() {
    vec3 color = texture(uniDownscaledBuffer, gl_FragCoord.xy * uniInvBufferSize).rgb;

    for (int i = 0; i < 9; ++i) {
      vec2 normalizedTexcoords = (gl_FragCoord.xy + kernelOffsets[i]) * uniInvBufferSize;
      color += texture(uniPrevUpscaledBuffer, normalizedTexcoords).rgb * kernelWeights[i];
    }

    fragColor = vec4(color, 1.0);
  }
)";

constexpr std::string_view final_source = R"(
  in vec2 fragTexcoords;

  uniform sampler2D uniOriginalColorBuffer;
  uniform sampler2D uniFinalUpscaledBuffer;

  layout(location = 0) out vec4 fragColor;

  void main() {
    vec3 originalColor = texture(uniOriginalColorBuffer, fragTexcoords).rgb;
    vec3 blurredColor  = texture(uniFinalUpscaledBuffer, fragTexcoords).rgb;

    // The following is technically incorrect, since tone mapping must be done on the whole scene at the very end of the rendering. This will be removed later
    blurredColor = blurredColor / (blurredColor + vec3(1.0)); // Tone mapping
    blurredColor = pow(blurredColor, vec3(1.0 / 2.2)); // Gamma correction

    fragColor = vec4(originalColor + blurredColor, 1.0);
  }
)";
}

namespace xen {
Bloom::Bloom(RenderGraph& render_graph) : RenderProcess(render_graph)
{
    // Based on Froyok's bloom, itself based on the one used in Unreal Engine 4/Call of Duty: Advanced Warfare
    // See: https://www.froyok.fr/blog/2021-12-ue4-custom-bloom/

    threshold_pass = &render_graph.add_node(FragmentShader::load_from_source(threshold_source), "Bloom thresholding");
    set_threshold_value(0.75f
    ); // Tone mapping is applied before the bloom, thus no value above 1 exist here. This value will be changed later

    auto const threshold_buffer = Texture2D::create(TextureColorspace::RGB, TextureDataType::FLOAT16);
    threshold_pass->add_write_color_texture(threshold_buffer, 0);

#if !defined(USE_OPENGL_ES)
    if (Renderer::check_version(4, 3)) {
        Renderer::set_label(
            RenderObjectType::PROGRAM, threshold_pass->get_program().get_index(), "Bloom threshold program"
        );
        Renderer::set_label(
            RenderObjectType::SHADER, threshold_pass->get_program().get_vertex_shader().get_index(),
            "Bloom threshold vertex shader"
        );
        Renderer::set_label(
            RenderObjectType::SHADER, threshold_pass->get_program().get_fragment_shader().get_index(),
            "Bloom threshold fragment shader"
        );
        Renderer::set_label(
            RenderObjectType::FRAMEBUFFER, threshold_pass->get_framebuffer().get_index(), "Bloom threshold framebuffer"
        );
        Renderer::set_label(RenderObjectType::TEXTURE, threshold_buffer->get_index(), "Bloom threshold buffer");
    }
#endif

    downscale_passes.resize(pass_count);
    downscale_buffers.resize(pass_count);

    for (size_t downscale_pass_index = 0; downscale_pass_index < pass_count; ++downscale_pass_index) {
        std::string const id_str = std::to_string(downscale_pass_index);

        RenderPass& downscale_pass =
            render_graph.add_node(FragmentShader::load_from_source(downscale_source), "Bloom downscale #" + id_str);

        //  ----------
        //  |        |
        //  |   D0   |
        //  |        |
        //  ----------
        //      |
        //      v prevDownscaledBuffer
        //    ------
        //    | D1 |
        //    ------
        //      |
        //      v prevDownscaledBuffer
        //     ...

        downscale_pass.add_read_texture(
            (downscale_pass_index == 0 ? threshold_buffer : downscale_buffers[downscale_pass_index - 1].lock()),
            "uniPrevDownscaledBuffer"
        );

        auto const downscaled_buffer = Texture2D::create(TextureColorspace::RGB, TextureDataType::FLOAT16);
        downscale_pass.add_write_color_texture(downscaled_buffer, 0);

        downscale_passes[downscale_pass_index] = &downscale_pass;
        downscale_buffers[downscale_pass_index] = downscaled_buffer;

        downscale_pass.add_parents(
            (downscale_pass_index == 0 ? *threshold_pass : *downscale_passes[downscale_pass_index - 1])
        );

#if !defined(USE_OPENGL_ES)
        if (Renderer::check_version(4, 3)) {
            Renderer::set_label(
                RenderObjectType::PROGRAM, downscale_pass.get_program().get_index(),
                "Bloom downscale program #" + id_str
            );
            Renderer::set_label(
                RenderObjectType::SHADER, downscale_pass.get_program().get_vertex_shader().get_index(),
                "Bloom downscale vertex shader #" + id_str
            );
            Renderer::set_label(
                RenderObjectType::SHADER, downscale_pass.get_program().get_fragment_shader().get_index(),
                "Bloom downscale fragment shader #" + id_str
            );
            Renderer::set_label(
                RenderObjectType::FRAMEBUFFER, downscale_pass.get_framebuffer().get_index(),
                "Bloom downscale framebuffer #" + id_str
            );
            Renderer::set_label(
                RenderObjectType::TEXTURE, downscaled_buffer->get_index(), "Bloom downscale buffer #" + id_str
            );
        }
#endif
    }

    upscale_passes.resize(pass_count - 1);
    upscale_buffers.resize(pass_count - 1);

    for (size_t upscale_pass_index = 0; upscale_pass_index < pass_count - 1; ++upscale_pass_index) {
        std::string const id_str = std::to_string(upscale_pass_index);

        RenderPass& upscale_pass =
            render_graph.add_node(FragmentShader::load_from_source(upscale_source), "Bloom upscale #" + id_str);

        //  ----------                ----------
        //  |        |                |        |
        //  |   D0   |--------------->|   U0   |
        //  |        |                |        |
        //  ----------                ----------
        //      |                         ^
        //      v                         |
        //    ------   downscaled_buffer ------
        //    | D1 |------------------->| U1 |
        //    ------                    ------
        //      |                         ^ prevUpscaledBuffer
        //      v                         |
        //     ... ------------------------

        auto const corresp_downscale_pass_index = pass_count - upscale_pass_index - 2;

        upscale_pass.add_read_texture(downscale_buffers[corresp_downscale_pass_index].lock(), "uniDownscaledBuffer");
        upscale_pass.add_read_texture(
            (upscale_pass_index == 0 ? downscale_buffers.back() : upscale_buffers[upscale_pass_index - 1]).lock(),
            "uniPrevUpscaledBuffer"
        );

        auto const upscaled_buffer = Texture2D::create(TextureColorspace::RGB, TextureDataType::FLOAT16);
        upscale_pass.add_write_color_texture(upscaled_buffer, 0);

        upscale_passes[upscale_pass_index] = &upscale_pass;
        upscale_buffers[upscale_pass_index] = upscaled_buffer;

        // Although each upscaling pass is technically dependent on the matching downscaling one, the render graph only
        // needs
        //  direct dependencies, that is, passes that can be executed anytime after their parents have been. In this
        //  case, we need to execute each one sequentially whenever the previous upscaling pass has finished anyway.
        //  Hence, although we could, we do not set any dependency between upscaling & downscaling passes aside from the
        //  first one

        upscale_pass.add_parents(
            (upscale_pass_index == 0 ? *downscale_passes.back() : *upscale_passes[upscale_pass_index - 1])
        );

#if !defined(USE_OPENGL_ES)
        if (Renderer::check_version(4, 3)) {
            Renderer::set_label(
                RenderObjectType::PROGRAM, upscale_pass.get_program().get_index(), "Bloom upscale program #" + id_str
            );
            Renderer::set_label(
                RenderObjectType::SHADER, upscale_pass.get_program().get_vertex_shader().get_index(),
                "Bloom upscale vertex shader #" + id_str
            );
            Renderer::set_label(
                RenderObjectType::SHADER, upscale_pass.get_program().get_fragment_shader().get_index(),
                "Bloom upscale fragment shader #" + id_str
            );
            Renderer::set_label(
                RenderObjectType::FRAMEBUFFER, upscale_pass.get_framebuffer().get_index(),
                "Bloom upscale framebuffer #" + id_str
            );
            Renderer::set_label(
                RenderObjectType::TEXTURE, upscaled_buffer->get_index(), "Bloom upscale buffer #" + id_str
            );
        }
#endif
    }

    final_pass = &render_graph.add_node(FragmentShader::load_from_source(final_source), "Bloom final pass");

    final_pass->add_parents(*upscale_passes.back());
    final_pass->add_read_texture(upscale_buffers.back().lock(), "uniFinalUpscaledBuffer");

#if !defined(USE_OPENGL_ES)
    if (Renderer::check_version(4, 3)) {
        Renderer::set_label(
            RenderObjectType::PROGRAM, final_pass->get_program().get_index(), "Bloom final pass program"
        );
        Renderer::set_label(
            RenderObjectType::SHADER, final_pass->get_program().get_vertex_shader().get_index(),
            "Bloom final pass vertex shader"
        );
        Renderer::set_label(
            RenderObjectType::SHADER, final_pass->get_program().get_fragment_shader().get_index(),
            "Bloom final pass fragment shader"
        );
    }
#endif

    // Validating the render graph
    if (!render_graph.is_valid())
        throw std::runtime_error("Error: The bloom process is invalid");
}

bool Bloom::is_enabled() const
{
    return threshold_pass->is_enabled();
}

void Bloom::set_state(bool const enabled)
{
    threshold_pass->enable(enabled);

    for (RenderPass* downscale_pass : downscale_passes)
        downscale_pass->enable(enabled);

    for (RenderPass* upscale_pass : upscale_passes)
        upscale_pass->enable(enabled);

    final_pass->enable(enabled);
}

void Bloom::add_parent(RenderPass& parent_pass)
{
    threshold_pass->add_parents(parent_pass);
}

void Bloom::add_parent(RenderProcess& parent_process)
{
    parent_process.add_child(*threshold_pass);
}

void Bloom::add_child(RenderPass& child_pass)
{
    final_pass->add_children(child_pass);
}

void Bloom::add_child(RenderProcess& child_process)
{
    child_process.add_parent(*final_pass);
}

void Bloom::resize_buffers(Vector2ui const& size)
{
    threshold_pass->resize_write_buffers(size);
    final_pass->resize_write_buffers(size);

    auto const downscale_buffers_size = downscale_buffers.size();
    auto const upscale_passes_size = upscale_passes.size();

    Vector2ui size_for_downscale = size;
    for (size_t i = 0; i < downscale_buffers_size; ++i) {
        size_for_downscale /= 2;

        Vector2f const inv_buffer_size(
            1.f / static_cast<float>(size_for_downscale.x), 1.f / static_cast<float>(size_for_downscale.y)
        );

        downscale_passes[i]->resize_write_buffers(size_for_downscale);

        downscale_passes[i]->get_program().set_attribute(inv_buffer_size, "uniInvBufferSize");
        downscale_passes[i]->get_program().send_attributes();

        if (i >= upscale_passes_size) {
            break;
        }

        auto const corresp_index = downscale_buffers_size - i - 2;

        upscale_passes[corresp_index]->resize_write_buffers(size_for_downscale);

        upscale_passes[corresp_index]->get_program().set_attribute(inv_buffer_size, "uniInvBufferSize");
        upscale_passes[corresp_index]->get_program().send_attributes();
    }
}

float Bloom::recover_elapsed_time() const
{
    float time = threshold_pass->recover_elapsed_time() + final_pass->recover_elapsed_time();

    for (RenderPass const* pass : downscale_passes) {
        time += pass->recover_elapsed_time();
    }

    for (RenderPass const* pass : upscale_passes) {
        time += pass->recover_elapsed_time();
    }

    return time;
}

void Bloom::set_input_color_buffer(Texture2DPtr color_buffer)
{
    resize_buffers(color_buffer->get_size());

    threshold_pass->add_read_texture(color_buffer, "uniColorBuffer");
    final_pass->add_read_texture(std::move(color_buffer), "uniOriginalColorBuffer");
}

void Bloom::set_output_buffer(Texture2DPtr output_buffer)
{
    final_pass->add_write_color_texture(std::move(output_buffer), 0);

#if !defined(USE_OPENGL_ES)
    if (Renderer::check_version(4, 3))
        Renderer::set_label(
            RenderObjectType::FRAMEBUFFER, final_pass->get_framebuffer().get_index(), "Bloom final pass framebuffer"
        );
#endif
}

void Bloom::set_threshold_value(float const threshold) const
{
    threshold_pass->get_program().set_attribute(threshold, "uniThreshold");
    threshold_pass->get_program().send_attributes();
}

}
