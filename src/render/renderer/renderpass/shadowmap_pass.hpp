#pragma once

#include <scene/components/camera.hpp>
#include <render/renderer/renderpass/render_pass.hpp>
#include <render/renderer/renderpass/render_pass_type.hpp>

namespace xen {
class Cubemap;
class Scene;
class Shader;
class Framebuffer;

class ShadowmapPass : public RenderPass {
private:
    std::shared_ptr<Shader> shadowmap_shader;
    std::shared_ptr<Shader> shadowmap_skinned_shader;
    std::shared_ptr<Shader> shadowmap_linear_shader;
    std::shared_ptr<Shader> shadowmap_linear_skinned_shader;

    CameraComponent camera;
    Framebuffer empty_framebuffer{Vector2ui(1, 1), false}; // Used for attaching to when rendering (like cubemap faces)

    // Option to use custom shadow framebuffers/cubemaps. Most will go through the light manager and request the
    // specified resolutions for normal rendering
    Framebuffer* custom_directional_light_shadow_framebuffer = nullptr;
    Framebuffer* custom_spot_light_shadow_framebuffer = nullptr;
    Cubemap* custom_point_light_shadow_cubemap = nullptr;

public:
    ShadowmapPass(Scene& scene);
    ShadowmapPass(
        Scene& scene, Framebuffer& custom_directional_light_shadow_framebuffer,
        Framebuffer& custom_spot_light_shadow_framebuffer, Cubemap& custom_point_light_shadow_cubemap
    );

    ShadowmapPassOutput generate_shadowmaps(CameraComponent& camera, bool render_only_static);

private:
    void init();
};
}