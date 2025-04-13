#pragma once

#include <render/renderer/renderpass/render_pass.hpp>
#include <render/renderer/renderpass/render_pass_type.hpp>

namespace xen {
class Shader;
class Scene;
class CameraComponent;
class Framebuffer;
class LightSystem;
class ProbeManager;

class ForwardLightingPass : public RenderPass {
private:
    std::shared_ptr<Framebuffer> framebuffer;
    std::shared_ptr<Shader> model_shader;
    std::shared_ptr<Shader> skinned_model_shader;
    std::shared_ptr<Shader> terrain_shader;

public:
    ForwardLightingPass(Scene& scene, bool should_multisample);
    ForwardLightingPass(Scene& scene, Framebuffer& custom_framebuffer);

    LightingPassOutput execute_opaque_lightning_pass(
        ShadowmapPassOutput& shadowmap_data, CameraComponent& camera, bool render_only_static, bool use_ibl
    );
    LightingPassOutput execute_transparent_lightning_pass(
        ShadowmapPassOutput& shadowmap_data, Framebuffer& framebuffer, CameraComponent& camera, bool render_only_static,
        bool use_ibl
    );

private:
    void init();

    void bind_shadowmap(Shader& shader, ShadowmapPassOutput& shadowmap_data);
};
}