#pragma once

#include <render/renderer/renderpass/render_pass.hpp>
#include <render/renderer/renderpass/render_pass_type.hpp>

namespace xen {
class Shader;
class Scene;
class CameraComponent;

class DeferredLightingPass : public RenderPass {
private:
    std::shared_ptr<Framebuffer> framebuffer;
    std::shared_ptr<Shader> lighting_shader;

public:
    DeferredLightingPass(Scene& scene);
    DeferredLightingPass(Scene& scene, Framebuffer& framebuffer);

    LightingPassOutput execute_lighting_pass(
        ShadowmapPassOutput& shadowmap_data, GBuffer& gbuffer, PreLightingPassOutput& pre_lighting_output,
        CameraComponent& camera, bool use_ibl
    );

private:
    void bind_shadowmap(Shader& shader, ShadowmapPassOutput& shadowmap_data);
};
}