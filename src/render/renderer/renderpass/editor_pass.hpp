#pragma once

#include <render/renderer/renderpass/render_pass.hpp>
#include <render/renderer/renderpass/render_pass_type.hpp>
#include <scene/entity.hpp>

namespace xen {
class Shader;
class Scene;
class CameraComponent;
class Framebuffer;

class EditorPass : public RenderPass {
private:
    std::shared_ptr<Shader> color_write_shader;
    std::shared_ptr<Shader> color_write_shader_skinned;
    std::shared_ptr<Shader> outline_shader;

    std::shared_ptr<Shader> unlit_sprite_shader;

    Entity focused_entity;

    // Editor textures
    Texture* directional_light_texture = nullptr;
    Texture* point_light_texture = nullptr;
    Texture* spot_light_texture = nullptr;

    // Shader tweaks
    float outline_size = 6.0f;
    Vector3f outline_color = Vector3f(0.68507f, 0.0f, 1.0f);

public:
    EditorPass(Scene& scene);

    EditorPassOutput execute(
        Framebuffer& scene_framebuffer, Framebuffer& extra_framebuffer1, Framebuffer& extra_framebuffer2,
        CameraComponent& camera
    );

    void set_focused_entity(Entity entity) { focused_entity = entity; }
};
}