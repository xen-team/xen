#include "post_process_pass.hpp"

#include <system/windows.hpp>
#include <render/shader.hpp>
#include <render/render.hpp>
#include <render/renderer/gl_cache.hpp>
#include <resource/resources.hpp>
#include <scene/scene.hpp>
#include <scene/components/camera.hpp>
#include <utils/shader_loader.hpp>

constexpr float half = 0.5f;
constexpr float quarter = 1.f / 4.f;
constexpr float eight = 1.f / 8.f;
constexpr float sixteen = 1.f / 16.f;
constexpr float thirty_two = 1.f / 32.f;
constexpr float sixty_four = 1.f / 64.f;

namespace xen {
PostProcessPass::PostProcessPass(Scene& scene) :
    RenderPass(scene), tonemap_gamma_correct_shader(ShaderLoader::load_shader("tonemap_gamma_correct.glsl")),
    fxaa_shader(ShaderLoader::load_shader("post_process/fxaa/fxaa.glsl")),
    ssao_shader(ShaderLoader::load_shader("post_process/ssao/ssao.glsl")),
    ssao_blur_shader(ShaderLoader::load_shader("post_process/ssao/ssao_blur.glsl")),
    bloom_bright_pass_shader(ShaderLoader::load_shader("post_process/bloom/bloom_bright_pass.glsl")),
    bloom_downsample_shader(ShaderLoader::load_shader("post_process/bloom/bloom_downsample.glsl")),
    bloom_upsample_shader(ShaderLoader::load_shader("post_process/bloom/bloom_upsample.glsl")),
    bloom_composite_shader(ShaderLoader::load_shader("post_process/bloom/bloom_composite.glsl")),
    vignette_shader(ShaderLoader::load_shader("post_process/vignette/vignette.glsl")),
    chromatic_aberration_shader(ShaderLoader::load_shader("post_process/chromatic_aberration/chromatic_aberration.glsl")
    ),
    film_grain_shader(ShaderLoader::load_shader("post_process/film_grain/film_grain.glsl")),

    ssao_render_target{Windows::get()->get_focused_window()->get_size() * half, false},
    ssao_blur_render_target{Windows::get()->get_focused_window()->get_size() * half, false},
    tonemapped_non_linear_target{Windows::get()->get_focused_window()->get_size(), false},
    resolve_render_target{Windows::get()->get_focused_window()->get_size(), false},
    bright_pass_render_target{Windows::get()->get_focused_window()->get_size(), false},
    bloom_half_render_target{Windows::get()->get_focused_window()->get_size() * half, false},
    bloom_quarter_render_target{Windows::get()->get_focused_window()->get_size() * quarter, false},
    bloom_eight_render_target{Windows::get()->get_focused_window()->get_size() * eight, false},
    bloom_sixteen_render_target{Windows::get()->get_focused_window()->get_size() * sixteen, false},
    bloom_thirty_two_render_target{Windows::get()->get_focused_window()->get_size() * thirty_two, false},
    bloom_sixty_four_render_target{Windows::get()->get_focused_window()->get_size() * sixty_four, false},
    full_render_target{Windows::get()->get_focused_window()->get_size(), false},
    half_render_target{Windows::get()->get_focused_window()->get_size() * half, false},
    quarter_render_target{Windows::get()->get_focused_window()->get_size() * quarter, false},
    eighth_render_target{Windows::get()->get_focused_window()->get_size() * eight, false}
{

    // Log::rt_assert(
    //     bloom_sixty_four_render_target.get_width() >= 1 && bloom_sixty_four_render_target.get_height() >= 1,
    //     "Render resolution is too low for bloom"
    // );

    // Framebuffer setup
    ssao_render_target.add_color_texture(ColorAttachmentFormat::NormalizedSingleChannel8).create_framebuffer();
    ssao_blur_render_target.add_color_texture(ColorAttachmentFormat::NormalizedSingleChannel8).create_framebuffer();
    tonemapped_non_linear_target.add_color_texture(ColorAttachmentFormat::Normalized8)
        .add_depth_stencil_rbo(DepthStencilAttachmentFormat::NormalizedDepthOnly)
        .create_framebuffer();
    resolve_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16)
        .add_depth_stencil_rbo(DepthStencilAttachmentFormat::NormalizedDepthOnly)
        .create_framebuffer();

    full_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    half_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    quarter_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    eighth_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();

    bright_pass_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    // m_BloomFullRenderTarget.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    bloom_half_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    bloom_quarter_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    bloom_eight_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    bloom_sixteen_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    bloom_thirty_two_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    bloom_sixty_four_render_target.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();

    // SSAO Hemisphere Sample Generation (tangent space)
    std::uniform_real_distribution<float> random_floats(0.0f, 1.0f);
    std::default_random_engine generator;
    for (size_t i = 0; i < ssao_kernel.size(); i++) {
        // Make sure that the samples aren't perfectly perpendicular to the normal, or depth reconstruction will yield
        // artifacts (so make sure the z value isn't close to 0)
        Vector3f hemisphere_sample = Vector3f(
            (random_floats(generator) * 2.0f) - 1.0f, (random_floats(generator) * 2.0f) - 1.0f,
            std::clamp(random_floats(generator), 0.2f, 1.0f)
        ); // Z = [0.2, 1] because we want hemisphere in tangent space
        hemisphere_sample = hemisphere_sample.normalize();

        // Generate more samples closer to the origin of the hemisphere. Since these make for better light occlusion
        // tests
        float scale = static_cast<float>(i / ssao_kernel.size());
        scale = std::lerp(0.1f, 1.0f, scale * scale);
        hemisphere_sample *= scale;

        ssao_kernel[i] = hemisphere_sample;
    }

    // SSAO Random Rotation Texture (used to apply a random rotation when constructing the change of basis matrix)
    // Random vectors should be in tangent space
    std::array<Vector3f, 16> noise_ssao;
    for (auto& noise : noise_ssao) {
        noise = Vector3f((random_floats(generator) * 2.0f) - 1.0f, (random_floats(generator) * 2.0f) - 1.0f, 0.0f);
    }

    TextureSettings ssao_noise_texture_settings;
    ssao_noise_texture_settings.texture_format = GL_RGB16F;
    ssao_noise_texture_settings.texture_wrap_s_mode = GL_REPEAT;
    ssao_noise_texture_settings.texture_wrap_t_mode = GL_REPEAT;
    ssao_noise_texture_settings.texture_minification_filter_mode = GL_NEAREST;
    ssao_noise_texture_settings.texture_magnification_filter_mode = GL_NEAREST;
    ssao_noise_texture_settings.texture_anisotropy_level = 1.0f;
    ssao_noise_texture_settings.has_mips = false;
    ssao_noise_texture.set_texture_settings(std::move(ssao_noise_texture_settings));
    ssao_noise_texture.generate_2d_texture(Vector2ui(4, 4), GL_RGB, GL_FLOAT, noise_ssao.data());
}

// Generates the AO of the scene using SSAO and stores it in a single channel texture
PreLightingPassOutput PostProcessPass::execute_pre_lighting_pass(GBuffer& input_gbuffer, CameraComponent& camera)
{
    PreLightingPassOutput pass_output;
    if (!ssao_enabled) {
        pass_output.ssao_texture = Resources::get_white_texture();
        return pass_output;
    }

    // Generate the AO factors for the scene
    // ARC_PUSH_RENDER_TAG("SSAO");
    GL_CALL(glViewport(0, 0, ssao_render_target.get_width(), ssao_render_target.get_height()));
    ssao_render_target.bind();
    gl_cache->set_depth_test(false);
    gl_cache->set_face_cull(true);
    gl_cache->set_cull_face(GL_BACK);

    // Bind the required data to perform SSAO
    gl_cache->set_shader(*ssao_shader);

    // Used to tile the noise texture across the screen every 4 texels (because our noise texture is 4x4)
    ssao_shader->set_uniform(
        "noiseScale", Vector2f(
                          static_cast<float>(ssao_render_target.get_width()) * 0.25f,
                          static_cast<float>(ssao_render_target.get_height()) * 0.25f
                      )
    );

    ssao_shader->set_uniform("ssaoStrength", ssao_strength);
    ssao_shader->set_uniform("sampleRadius", ssao_sample_radius);
    ssao_shader->set_uniform("sampleRadius2", ssao_sample_radius * ssao_sample_radius);
    ssao_shader->set_uniform("numKernelSamples", static_cast<int>(ssao_kernel.size()));
    ssao_shader->set_uniform_array("samples", ssao_kernel);

    ssao_shader->set_uniform("view", camera.get_view());
    ssao_shader->set_uniform("projection", camera.get_projection());
    ssao_shader->set_uniform("viewInverse", camera.get_view().inverse());
    ssao_shader->set_uniform("projectionInverse", camera.get_projection().inverse());

    input_gbuffer.get_normal()->bind(0);
    ssao_shader->set_uniform("normalTexture", 0);
    input_gbuffer.get_depth_stencil_texture().bind(1);
    ssao_shader->set_uniform("depthTexture", 1);
    ssao_noise_texture.bind(2);
    ssao_shader->set_uniform("texNoise", 2);

    // Render our NDC quad to perform SSAO
    Render::get()->get_renderer()->draw_ndc_plane();

    // Blur the result
    ssao_blur_render_target.bind();
    ssao_blur_shader->enable();

    ssao_blur_shader->set_uniform("numSamplesAroundTexel", 2); // 5x5 kernel blur
    ssao_blur_shader->set_uniform("ssaoInput", 0);             // Texture unit
    ssao_render_target.get_color_texture().bind(0);

    // Render our NDC quad to blur our SSAO texture
    Render::get()->get_renderer()->draw_ndc_plane();
    // ARC_POP_RENDER_TAG();

    // Reset unusual state
    gl_cache->set_depth_test(true);

    // Render pass output
    pass_output.ssao_texture = &ssao_blur_render_target.get_color_texture();
    return pass_output;
}

PostProcessPassOutput PostProcessPass::execute_post_process_pass(Framebuffer& framebuffer_to_process)
{
    PostProcessPassOutput output;

    // GLCache& gl_cache = GLCache::get();

    // If the framebuffer is multi-sampled, resolve it
    Framebuffer* input_framebuffer = &framebuffer_to_process;
    if (framebuffer_to_process.is_multisampled()) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_to_process.get_framebuffer());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_render_target.get_framebuffer());
        glBlitFramebuffer(
            0, 0, static_cast<GLsizei>(framebuffer_to_process.get_width()),
            static_cast<GLsizei>(framebuffer_to_process.get_height()), 0, 0,
            static_cast<GLsizei>(resolve_render_target.get_width()),
            static_cast<GLsizei>(resolve_render_target.get_height()), GL_COLOR_BUFFER_BIT, GL_NEAREST
        );
        input_framebuffer = &resolve_render_target;
    }

    // Wireframe code otherwise we will just render a quad in wireframe
    // if (Application::GetInstance().GetWireframe()) {
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // }

    // Apply bloom if enabled, then Convert our scene from HDR (linear) -> SDR (sRGB) regardless if we apply bloom or
    // not
    if (bloom_enabled) {
        auto& scene_with_bloom = bloom(input_framebuffer->get_color_texture());
        tonemap_gamma_correct(tonemapped_non_linear_target, scene_with_bloom);
    }
    else {
        tonemap_gamma_correct(tonemapped_non_linear_target, input_framebuffer->get_color_texture());
    }

    input_framebuffer = &tonemapped_non_linear_target;

    // Now apply various post processing effects after we are in SDR
    Framebuffer* framebuffer_to_render_to = nullptr;
    if (chromatic_aberration_enabled) {
        if (framebuffer_to_render_to == &full_render_target) {
            framebuffer_to_render_to = &tonemapped_non_linear_target;
        }
        else {
            framebuffer_to_render_to = &full_render_target;
        }

        chromatic_aberration(*framebuffer_to_render_to, input_framebuffer->get_color_texture());
        input_framebuffer = framebuffer_to_render_to;
    }

    if (film_grain_enabled) {
        if (framebuffer_to_render_to == &full_render_target) {
            framebuffer_to_render_to = &tonemapped_non_linear_target;
        }
        else {
            framebuffer_to_render_to = &full_render_target;
        }

        film_grain(*framebuffer_to_render_to, input_framebuffer->get_color_texture());
        input_framebuffer = framebuffer_to_render_to;
    }

    if (vignette_enabled) {
        if (framebuffer_to_render_to == &full_render_target) {
            framebuffer_to_render_to = &tonemapped_non_linear_target;
        }
        else {
            framebuffer_to_render_to = &full_render_target;
        }

        if (vignette_texture && vignette_texture->is_generated()) {
            vignette(*framebuffer_to_render_to, input_framebuffer->get_color_texture(), vignette_texture);
        }
        else {
            vignette(*framebuffer_to_render_to, input_framebuffer->get_color_texture());
        }
        input_framebuffer = framebuffer_to_render_to;
    }

    if (fxaa_enabled) {
        if (framebuffer_to_render_to == &full_render_target) {
            framebuffer_to_render_to = &tonemapped_non_linear_target;
        }
        else {
            framebuffer_to_render_to = &full_render_target;
        }

        fxaa(*framebuffer_to_render_to, input_framebuffer->get_color_texture());
        input_framebuffer = framebuffer_to_render_to;
    }

    // Finally return the output frame after being post processed
    output.out_framebuffer = input_framebuffer;
    return output;
}

void PostProcessPass::tonemap_gamma_correct(Framebuffer& target, Texture& hdr_texture)
{
    // ARC_PUSH_RENDER_TAG("Tonemap & Gamma Correct");
    GL_CALL(glViewport(0, 0, static_cast<GLsizei>(target.get_width()), static_cast<GLsizei>(target.get_height())));
    gl_cache->set_shader(*tonemap_gamma_correct_shader);
    gl_cache->set_depth_test(false);
    gl_cache->set_blend(false);
    gl_cache->set_face_cull(true);
    gl_cache->set_cull_face(GL_BACK);
    gl_cache->set_stencil_test(false);
    target.bind();

    tonemap_gamma_correct_shader->set_uniform("gamma_inverse", 1.0f / gamma_correction);
    tonemap_gamma_correct_shader->set_uniform("exposure", exposure);
    tonemap_gamma_correct_shader->set_uniform("input_texture", 0);
    hdr_texture.bind(0);

    Render::get()->get_renderer()->draw_ndc_plane();
    // ARC_POP_RENDER_TAG();
}

void PostProcessPass::fxaa(Framebuffer& target, Texture& texture)
{
    // ARC_PUSH_RENDER_TAG("FXAA");
    GL_CALL(glViewport(0, 0, static_cast<GLsizei>(target.get_width()), static_cast<GLsizei>(target.get_height())));
    gl_cache->set_shader(*fxaa_shader);
    gl_cache->set_depth_test(false);
    gl_cache->set_blend(false);
    gl_cache->set_face_cull(true);
    gl_cache->set_cull_face(GL_BACK);
    gl_cache->set_stencil_test(false);
    target.bind();

    fxaa_shader->set_uniform(
        "texel_size",
        Vector2f(1.0f / static_cast<float>(texture.get_width()), 1.0f / static_cast<float>(texture.get_height()))
    );
    fxaa_shader->set_uniform("input_texture", 0);
    texture.bind(0);

    Render::get()->get_renderer()->draw_ndc_plane();
    // ARC_POP_RENDER_TAG();
}

void PostProcessPass::vignette(Framebuffer& target, Texture& texture, Texture* optional_vignette_mask)
{
    // ARC_PUSH_RENDER_TAG("Vignette");
    GL_CALL(glViewport(0, 0, static_cast<GLsizei>(target.get_width()), static_cast<GLsizei>(target.get_height())));
    gl_cache->set_shader(*vignette_shader);
    gl_cache->set_depth_test(false);
    gl_cache->set_blend(false);
    gl_cache->set_face_cull(true);
    gl_cache->set_cull_face(GL_BACK);
    gl_cache->set_stencil_test(false);
    target.bind();

    vignette_shader->set_uniform("colour", vignette_color);
    vignette_shader->set_uniform("intensity", vignette_intensity);
    vignette_shader->set_uniform("input_texture", 0);
    texture.bind(0);
    if (optional_vignette_mask != nullptr) {
        vignette_shader->set_uniform("usesMask", 1);
        vignette_shader->set_uniform("vignette_mask", 1);
        optional_vignette_mask->bind(1);
    }

    Render::get()->get_renderer()->draw_ndc_plane();
    // ARC_POP_RENDER_TAG();
}

void PostProcessPass::chromatic_aberration(Framebuffer& target, Texture& texture)
{
    // ARC_PUSH_RENDER_TAG("Chromatic Aberration");
    GL_CALL(glViewport(0, 0, static_cast<GLsizei>(target.get_width()), static_cast<GLsizei>(target.get_height())));
    gl_cache->set_shader(*chromatic_aberration_shader);
    gl_cache->set_depth_test(false);
    gl_cache->set_blend(false);
    gl_cache->set_face_cull(true);
    gl_cache->set_cull_face(GL_BACK);
    gl_cache->set_stencil_test(false);
    target.bind();

    chromatic_aberration_shader->set_uniform("intensity", chromatic_aberration_intensity * 100);
    chromatic_aberration_shader->set_uniform(
        "texel_size",
        Vector2f(1.0f / static_cast<float>(texture.get_width()), 1.0f / static_cast<float>(texture.get_height()))
    );
    chromatic_aberration_shader->set_uniform("input_texture", 0);
    texture.bind(0);

    Render::get()->get_renderer()->draw_ndc_plane();
    // ARC_POP_RENDER_TAG();
}

void PostProcessPass::film_grain(Framebuffer& target, Texture& texture)
{
    // ARC_PUSH_RENDER_TAG("Film Grain");
    GL_CALL(glViewport(0, 0, static_cast<GLsizei>(target.get_width()), static_cast<GLsizei>(target.get_height())));
    gl_cache->set_shader(*film_grain_shader);
    gl_cache->set_depth_test(false);
    gl_cache->set_blend(false);
    gl_cache->set_face_cull(true);
    gl_cache->set_cull_face(GL_BACK);
    gl_cache->set_stencil_test(false);
    target.bind();

    film_grain_shader->set_uniform("intensity", film_grain_intensity * 100.0f);
    film_grain_shader->set_uniform("time", (float)(std::fmod(effects_timer.elapsed(), 100.0)));
    film_grain_shader->set_uniform("input_texture", 0);
    texture.bind(0);

    Render::get()->get_renderer()->draw_ndc_plane();
    // ARC_POP_RENDER_TAG();
}

// https://www.youtube.com/watch?v=ml-5OGZC7vE
// Great summary of the advanced warfare bloom talk and what Arcane's implementation is based on
Texture& PostProcessPass::bloom(Texture& hdr_scene_texture)
{
    // ARC_PUSH_RENDER_TAG("Bloom");
    gl_cache->set_depth_test(false);
    gl_cache->set_blend(false);
    gl_cache->set_face_cull(true);
    gl_cache->set_cull_face(GL_BACK);
    gl_cache->set_stencil_test(false);

    // Bloom Bright Pass
    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bright_pass_render_target.get_width()),
        static_cast<GLsizei>(bright_pass_render_target.get_height())
    ));

    bright_pass_render_target.bind();
    bright_pass_render_target.clear_all();
    gl_cache->set_shader(*bloom_bright_pass_shader);
    Vector4f filter_values;
    float const knee = bloom_threshold * bloom_soft_threshold;
    filter_values.x = bloom_threshold;
    filter_values.y = filter_values.x - knee;
    filter_values.z = 2.0f * knee;
    filter_values.w = 0.25f / (knee + 0.00001f);
    bloom_bright_pass_shader->set_uniform("filterValues", filter_values);
    bloom_bright_pass_shader->set_uniform("sceneCapture", 0);
    hdr_scene_texture.bind(0);
    Render::get()->get_renderer()->draw_ndc_plane();

    // Downsampling the parts of the scene that are above the luminance threshold using a 13 tap bilinear
    // filter (Kawase downsample style)
    gl_cache->set_shader(*bloom_downsample_shader);
    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_half_render_target.get_width()),
        static_cast<GLsizei>(bloom_half_render_target.get_height())
    ));

    bloom_half_render_target.bind();
    bloom_half_render_target.clear_all();
    bloom_downsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_half_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_half_render_target.get_height())
                     )
    );
    bloom_downsample_shader->set_uniform("textureToDownsample", 0);
    bright_pass_render_target.get_color_texture().bind(0);
    Render::get()->get_renderer()->draw_ndc_plane();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_quarter_render_target.get_width()),
        static_cast<GLsizei>(bloom_quarter_render_target.get_height())
    ));

    bloom_quarter_render_target.bind();
    bloom_quarter_render_target.clear_all();
    bloom_downsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_quarter_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_quarter_render_target.get_height())
                     )
    );
    bloom_downsample_shader->set_uniform("textureToDownsample", 0);
    bloom_half_render_target.get_color_texture().bind(0);
    Render::get()->get_renderer()->draw_ndc_plane();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_eight_render_target.get_width()),
        static_cast<GLsizei>(bloom_eight_render_target.get_height())
    ));

    bloom_eight_render_target.bind();
    bloom_eight_render_target.clear_all();
    bloom_downsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_eight_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_eight_render_target.get_height())
                     )
    );
    bloom_downsample_shader->set_uniform("textureToDownsample", 0);
    bloom_quarter_render_target.get_color_texture().bind(0);
    Render::get()->get_renderer()->draw_ndc_plane();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_sixteen_render_target.get_width()),
        static_cast<GLsizei>(bloom_sixteen_render_target.get_height())
    ));

    bloom_sixteen_render_target.bind();
    bloom_sixteen_render_target.clear_all();
    bloom_downsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_sixteen_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_sixteen_render_target.get_height())
                     )
    );
    bloom_downsample_shader->set_uniform("textureToDownsample", 0);
    bloom_eight_render_target.get_color_texture().bind(0);
    Render::get()->get_renderer()->draw_ndc_plane();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_thirty_two_render_target.get_width()),
        static_cast<GLsizei>(bloom_thirty_two_render_target.get_height())
    ));

    bloom_thirty_two_render_target.bind();
    bloom_thirty_two_render_target.clear_all();
    bloom_downsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_thirty_two_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_thirty_two_render_target.get_height())
                     )
    );
    bloom_downsample_shader->set_uniform("textureToDownsample", 0);
    bloom_sixteen_render_target.get_color_texture().bind(0);
    Render::get()->get_renderer()->draw_ndc_plane();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_sixty_four_render_target.get_width()),
        static_cast<GLsizei>(bloom_sixty_four_render_target.get_height())
    ));

    bloom_sixty_four_render_target.bind();
    bloom_sixty_four_render_target.clear_all();
    bloom_downsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_sixty_four_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_sixty_four_render_target.get_height())
                     )
    );
    bloom_downsample_shader->set_uniform("textureToDownsample", 0);
    bloom_thirty_two_render_target.get_color_texture().bind(0);
    Render::get()->get_renderer()->draw_ndc_plane();

    // Upsampling using a 9 tap tent bilinear filter to get back to high res
    gl_cache->set_shader(*bloom_upsample_shader);
    gl_cache->set_blend(true);
    gl_cache->set_blend_func(GL_ONE, GL_ONE);

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_thirty_two_render_target.get_width()),
        static_cast<GLsizei>(bloom_thirty_two_render_target.get_height())
    ));

    bloom_thirty_two_render_target.bind();
    bloom_upsample_shader->set_uniform("sampleScale", Vector4f(1.0, 1.0, 1.0, 1.0));
    bloom_upsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_thirty_two_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_thirty_two_render_target.get_height())
                     )
    );
    bloom_upsample_shader->set_uniform("textureToUpsample", 0);
    bloom_sixty_four_render_target.get_color_texture().bind(0);

    Render::get()->get_renderer()->draw_ndc_plane();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_sixteen_render_target.get_width()),
        static_cast<GLsizei>(bloom_sixteen_render_target.get_height())
    ));

    bloom_sixteen_render_target.bind();
    bloom_upsample_shader->set_uniform("sampleScale", Vector4f(1.0, 1.0, 1.0, 1.0));
    bloom_upsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_sixteen_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_sixteen_render_target.get_height())
                     )
    );

    bloom_upsample_shader->set_uniform("textureToUpsample", 0);
    bloom_thirty_two_render_target.get_color_texture().bind();

    Render::get()->get_renderer()->draw_ndc_plane();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_eight_render_target.get_width()),
        static_cast<GLsizei>(bloom_eight_render_target.get_height())
    ));

    bloom_eight_render_target.bind();
    bloom_upsample_shader->set_uniform("sampleScale", Vector4f(1.0, 1.0, 1.0, 1.0));
    bloom_upsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_eight_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_eight_render_target.get_height())
                     )
    );
    bloom_upsample_shader->set_uniform("textureToUpsample", 0);
    bloom_sixteen_render_target.get_color_texture().bind();

    Render::get()->get_renderer()->draw_ndc_plane();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_quarter_render_target.get_width()),
        static_cast<GLsizei>(bloom_quarter_render_target.get_height())
    ));

    bloom_quarter_render_target.bind();
    bloom_upsample_shader->set_uniform("sampleScale", Vector4f(1.0, 1.0, 1.0, 1.0));
    bloom_upsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_quarter_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_quarter_render_target.get_height())
                     )
    );
    bloom_upsample_shader->set_uniform("textureToUpsample", 0);
    bloom_eight_render_target.get_color_texture().bind();

    Render::get()->get_renderer()->draw_ndc_plane();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(bloom_half_render_target.get_width()),
        static_cast<GLsizei>(bloom_half_render_target.get_height())
    ));

    bloom_half_render_target.bind();
    bloom_upsample_shader->set_uniform("sampleScale", Vector4f(1.0, 1.0, 1.0, 1.0));
    bloom_upsample_shader->set_uniform(
        "texelSize", Vector2f(
                         1.0f / static_cast<float>(bloom_half_render_target.get_width()),
                         1.0f / static_cast<float>(bloom_half_render_target.get_height())
                     )
    );
    bloom_upsample_shader->set_uniform("textureToUpsample", 0);
    bloom_quarter_render_target.get_color_texture().bind();

    Render::get()->get_renderer()->draw_ndc_plane();

    // Combine our bloom texture with the scene
    gl_cache->set_blend(false);
    gl_cache->set_shader(*bloom_composite_shader);

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(full_render_target.get_width()),
        static_cast<GLsizei>(full_render_target.get_height())
    ));

    full_render_target.bind();
    bloom_composite_shader->set_uniform("bloomStrength", bloom_strength);
    bloom_composite_shader->set_uniform("dirtMaskIntensity", bloom_dirt_mask_intensity);
    bloom_composite_shader->set_uniform("sceneTexture", 0);
    bloom_composite_shader->set_uniform("bloomTexture", 1);
    bloom_composite_shader->set_uniform("dirtMaskTexture", 2);
    hdr_scene_texture.bind(0);
    bloom_half_render_target.get_color_texture().bind(1);

    if (bloom_dirt_texture && bloom_dirt_texture->is_generated()) {
        bloom_dirt_texture->bind(2);
    }
    else {
        Resources::get_black_texture()->bind(2);
    }

    Render::get()->get_renderer()->draw_ndc_plane();
    // ARC_POP_RENDER_TAG();

    return full_render_target.get_color_texture();
}
}