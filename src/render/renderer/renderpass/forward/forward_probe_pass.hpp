#pragma once

#include <render/texture/cubemap.hpp>
#include <render/renderer/renderpass/render_pass.hpp>
#include <scene/components/camera.hpp>
#include <platform/opengl/framebuffer.hpp>

namespace xen {
class Shader;
class Scene;

class ForwardProbePass : public RenderPass {
private:
    Framebuffer scene_capture_dir_light_shadow_framebuffer;
    Framebuffer scene_capture_spot_lights_shadow_framebuffer;
    Framebuffer scene_capture_lighting_framebuffer;
    Framebuffer light_probe_convolution_framebuffer;
    Framebuffer reflection_probe_sampling_framebuffer;

    Cubemap scene_capture_point_light_depth_cubemap;
    CameraComponent camera;
    CubemapSettings scene_capture_settings;
    Cubemap scene_capture_cubemap;

    std::shared_ptr<Shader> convolution_shader;
    std::shared_ptr<Shader> importance_sampling_shader;

public:
    ForwardProbePass(Scene& scene);

    void pregenerate_ibl();
    void pregenerate_probes();

    void generate_light_probe(Vector3f const& probe_position);
    void generate_reflection_probe(Vector3f const& probe_position);

private:
    void generate_brdflut();
    void generate_fallback_probes();
};
}