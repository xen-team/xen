#pragma once

#include <render/renderer/renderpass/render_pass.hpp>
#include <render/renderer/renderpass/render_pass_type.hpp>
#include <platform/opengl/framebuffer.hpp>
#include <utils/timers.hpp>

namespace xen {
class Shader;
class Scene;
class CameraComponent;

class PostProcessPass : public RenderPass {
private:
    std::shared_ptr<Shader> tonemap_gamma_correct_shader;
    std::shared_ptr<Shader> fxaa_shader;
    std::shared_ptr<Shader> ssao_shader;
    std::shared_ptr<Shader> ssao_blur_shader;

    std::shared_ptr<Shader> bloom_bright_pass_shader;
    std::shared_ptr<Shader> bloom_downsample_shader;
    std::shared_ptr<Shader> bloom_upsample_shader;
    std::shared_ptr<Shader> bloom_composite_shader;

    std::shared_ptr<Shader> vignette_shader;
    std::shared_ptr<Shader> chromatic_aberration_shader;
    std::shared_ptr<Shader> film_grain_shader;

    Framebuffer ssao_render_target;
    Framebuffer ssao_blur_render_target;
    Framebuffer tonemapped_non_linear_target;
    Framebuffer resolve_render_target; // Only used if multi-sampling is enabled so it can be resolved

    Texture* bloom_dirt_texture = nullptr;
    Framebuffer bright_pass_render_target;
    Framebuffer bloom_half_render_target;
    Framebuffer bloom_quarter_render_target;
    Framebuffer bloom_eight_render_target;
    Framebuffer bloom_sixteen_render_target;
    Framebuffer bloom_thirty_two_render_target;
    Framebuffer bloom_sixty_four_render_target;

    // Utility Framebuffers
    Framebuffer full_render_target;
    Framebuffer half_render_target;
    Framebuffer quarter_render_target;
    Framebuffer eighth_render_target;

    // Post Processing Tweaks
    float gamma_correction = 2.2f;
    float exposure = 1.0f;
    bool bloom_enabled = true;
    float bloom_threshold = 3.0f;
    float bloom_soft_threshold = 0.5f; // [0, 1] 0 = hard cutoff, 1 = soft cutoff between bloom vs no bloom
    float bloom_strength = 0.4f;
    float bloom_dirt_mask_intensity = 5.0f;
    bool fxaa_enabled = true;
    bool ssao_enabled = true;
    float ssao_sample_radius = 2.0f;
    float ssao_strength = 3.0f;
    bool vignette_enabled = false;
    Texture* vignette_texture = nullptr;
    Vector3f vignette_color = Vector3f(0.0f, 0.0f, 0.0f);
    float vignette_intensity = 0.25f;
    bool chromatic_aberration_enabled = false;
    float chromatic_aberration_intensity = 0.25f;
    bool film_grain_enabled = false;
    float film_grain_intensity = 0.25f;

    // SSAO Tweaks
    std::array<Vector3f, ssao_kernel_size> ssao_kernel;
    Texture ssao_noise_texture;

    SimpleTimer effects_timer;

public:
    PostProcessPass(Scene& scene);

    PreLightingPassOutput execute_pre_lighting_pass(GBuffer& input_gbuffer, CameraComponent& camera);
    PostProcessPassOutput execute_post_process_pass(Framebuffer& framebuffer_to_process);

    // Post Processing Effects
    void tonemap_gamma_correct(Framebuffer& target, Texture& hdr_texture);
    void fxaa(Framebuffer& target, Texture& texture);
    void vignette(Framebuffer& target, Texture& texture, Texture* optional_vignette_mask = nullptr);
    void chromatic_aberration(Framebuffer& target, Texture& texture);
    void film_grain(Framebuffer& target, Texture& texture);
    Texture& bloom(Texture& hdr_scene_texture);

    // Tonemap bindings
    [[nodiscard]] float& get_gamma_correction() { return gamma_correction; }
    [[nodiscard]] float& get_exposure() { return exposure; }

    // Bloom bindings
    [[nodiscard]] bool& get_bloom_enabled() { return bloom_enabled; }
    [[nodiscard]] float& get_bloom_threshold() { return bloom_threshold; }
    [[nodiscard]] float& get_bloom_soft_threshold() { return bloom_soft_threshold; }
    [[nodiscard]] float& get_bloom_strength() { return bloom_strength; }
    [[nodiscard]] Texture* get_bloom_dirt_texture() { return bloom_dirt_texture; }
    [[nodiscard]] float& get_bloom_dirt_mask_intensity() { return bloom_dirt_mask_intensity; }

    // SSAO bindings
    [[nodiscard]] bool& get_ssao_enabled() { return ssao_enabled; }
    [[nodiscard]] float& get_ssao_sample_radius() { return ssao_sample_radius; }
    [[nodiscard]] float& get_ssao_strength() { return ssao_strength; }

    // FXAA bindings
    [[nodiscard]] bool& get_fxaa_enabled() { return fxaa_enabled; }

    // Vignette bindings
    [[nodiscard]] bool& get_vignette_enabled() { return vignette_enabled; }
    [[nodiscard]] Texture* get_vignette_texture() { return vignette_texture; }
    [[nodiscard]] Vector3f& get_vignette_color() { return vignette_color; }
    [[nodiscard]] float& get_vignette_intensity() { return vignette_intensity; }

    // Chromatic Aberration bindings
    [[nodiscard]] bool& get_chromatic_aberration_enabled() { return chromatic_aberration_enabled; }
    [[nodiscard]] float& get_chromatic_aberration_intensity() { return chromatic_aberration_intensity; }

    // Film Grain bindings
    [[nodiscard]] bool& get_film_grain_enabled() { return film_grain_enabled; }
    [[nodiscard]] float& get_film_grain_intensity() { return film_grain_intensity; }

    // Render Target Access (TODO: Should use render target aliasing and have a system for sharing render targets for
    // different render passes. But this will suffice for now) Silly to manage all of these like this
    [[nodiscard]] Framebuffer& get_full_render_target() { return full_render_target; }
    [[nodiscard]] Framebuffer& get_half_render_target() { return half_render_target; }
    [[nodiscard]] Framebuffer& get_quarter_render_target() { return quarter_render_target; }
    [[nodiscard]] Framebuffer& get_eighth_render_target() { return eighth_render_target; }
    [[nodiscard]] Framebuffer& get_resolve_render_target() { return resolve_render_target; }
    [[nodiscard]] Framebuffer& get_tonemapped_non_linear_target() { return tonemapped_non_linear_target; }

    // Bloom settings
    void set_bloom_dirt_texture(Texture& texture) { bloom_dirt_texture = &texture; }
    void set_bloom_dirt_mask_intensity(float intensity) { bloom_dirt_mask_intensity = intensity; }

    // Vignette settings
    void set_vignette_texture(Texture& texture) { vignette_texture = &texture; }
};
}