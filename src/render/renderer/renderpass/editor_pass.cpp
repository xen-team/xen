#include "editor_pass.hpp"

#include <render/shader.hpp>
#include <render/render.hpp>
#include <render/renderer/gl_cache.hpp>
#include <utils/shader_loader.hpp>
#include <resource/resources.hpp>
#include <scene/scene.hpp>
#include <scene/components/camera.hpp>
#include <scene/components/mesh.hpp>
#include <scene/components/pose_animator.hpp>
#include <scene/components/light.hpp>

namespace xen {
EditorPass::EditorPass(Scene& scene) :
    RenderPass(scene), color_write_shader{ShaderLoader::load_shader("color_write.glsl")},
    color_write_shader_skinned{ShaderLoader::load_shader("color_write_skinned.glsl")},
    outline_shader{ShaderLoader::load_shader("outline.glsl")},
    unlit_sprite_shader{ShaderLoader::load_shader("2d/unlit_sprite.glsl")},
    directional_light_texture{Resources::get()->load_2d_texture_async("res/editor/directional_light.png")},
    point_light_texture{Resources::get()->load_2d_texture_async("res/editor/point_light.png")},
    spot_light_texture{Resources::get()->load_2d_texture_async("res/editor/spot_light.png")}
{
}

EditorPassOutput EditorPass::execute(
    Framebuffer& scene_framebuffer, Framebuffer& extra_framebuffer1, Framebuffer& extra_framebuffer2,
    CameraComponent& camera
)
{
    EditorPassOutput output;
    output.out_framebuffer = &scene_framebuffer;

    // Entity highlighting (should be done first since it might use debug rendering to highlight objects if no mesh
    // exists to highlight)
    // ARC_PUSH_RENDER_TAG("Entity Highlighting");
    if (focused_entity.is_valid() && focused_entity.has_component<MeshComponent>()) {
        auto& mesh_component = focused_entity.get_component<MeshComponent>();
        auto& transform_component = focused_entity.get_component<TransformComponent>();

        PoseAnimator* pose_animator = nullptr;
        if (focused_entity.has_component<PoseAnimatorComponent>()) {
            pose_animator = &focused_entity.get_component<PoseAnimatorComponent>().pose_animator;
        }

        glViewport(
            0, 0, static_cast<GLsizei>(extra_framebuffer1.get_width()),
            static_cast<GLsizei>(extra_framebuffer1.get_height())
        );
        extra_framebuffer1.bind();
        extra_framebuffer1.clear_all();

        // Setup initial state
        gl_cache->set_depth_test(false);
        gl_cache->set_stencil_test(false);
        gl_cache->set_blend(false);
        gl_cache->set_multisample(false);

        // Add objects that need to be outlined to the renderer (make them opaque so no sorting is done while we are
        // writing to our outline shader)
        Render::get()->get_renderer()->queue_mesh(
            mesh_component.model, transform_component.transform(), pose_animator, false,
            mesh_component.should_backface_cull
        );

        // Finally render our meshes (skinned and non-skinned)
        {
            gl_cache->set_shader(*color_write_shader_skinned);
            color_write_shader_skinned->set_uniform("colour", Vector3f(1.f, 1.f, 1.f));
            Render::get()->get_renderer()->flush_opaque_skinned_meshes(
                camera, RenderPassType::NoMaterialRequired, *color_write_shader_skinned
            );
        }
        {
            gl_cache->set_shader(*color_write_shader);
            color_write_shader->set_uniform("colour", Vector3f(1.f, 1.f, 1.f));
            Render::get()->get_renderer()->flush_opaque_non_skinned_meshes(
                camera, RenderPassType::NoMaterialRequired, *color_write_shader
            );
        }

        // Combine the objects that need to be highlighted with the scene to get the final output
        GL_CALL(glViewport(
            0, 0, static_cast<GLsizei>(extra_framebuffer2.get_width()),
            static_cast<GLsizei>(extra_framebuffer2.get_height())
        ));

        extra_framebuffer2.bind();
        extra_framebuffer2.clear_all();

        gl_cache->set_shader(*outline_shader);
        outline_shader->set_uniform("outlineSize", outline_size);
        outline_shader->set_uniform("outlineColour", outline_color);
        outline_shader->set_uniform("sceneTexture", 0);
        scene_framebuffer.get_color_texture().bind(0);
        outline_shader->set_uniform("highlightTexture", 1);
        extra_framebuffer1.get_color_texture().bind(1);
        Render::get()->get_renderer()->draw_ndc_plane();

        // Reset state
        gl_cache->set_depth_test(true);

        output.out_framebuffer = &extra_framebuffer2; // Update the output framebuffer
    }
    else if (focused_entity.is_valid() && focused_entity.has_component<TransformComponent>()) {
        // auto& transform_component = focused_entity.get_component<TransformComponent>();

        // DebugDraw3D::QueueBox(transform_component.position, transform_component.scale, outline_color);
    }
    // ARC_POP_RENDER_TAG();

    // DebugDraw3D
    // ARC_PUSH_RENDER_TAG("Debug 3D Draw");
    {
        GL_CALL(glViewport(
            0, 0, static_cast<GLsizei>(output.out_framebuffer->get_width()),
            static_cast<GLsizei>(output.out_framebuffer->get_height())
        ));

        output.out_framebuffer->bind();

        // Setup state
        gl_cache->set_depth_test(false);
        gl_cache->set_stencil_test(false);
        gl_cache->set_blend(false);
        gl_cache->set_multisample(false);

        // DebugDraw3D::FlushBatch(camera);
    }
    // ARC_POP_RENDER_TAG();

    // Debug Light Drawing (can clear depth so do this last)
    // ARC_PUSH_RENDER_TAG("Light Sprites");
    {
        GL_CALL(glViewport(
            0, 0, static_cast<GLsizei>(output.out_framebuffer->get_width()),
            static_cast<GLsizei>(output.out_framebuffer->get_height())
        ));

        output.out_framebuffer->bind();
        output.out_framebuffer->clear_depth(); // Clear depth, not needed and might cause our quad not to render

        // Setup state
        gl_cache->set_depth_test(false);
        gl_cache->set_stencil_test(false);
        gl_cache->set_blend(false);
        gl_cache->set_multisample(false);

        gl_cache->set_shader(*unlit_sprite_shader);
        unlit_sprite_shader->set_uniform("view", camera.get_view());
        unlit_sprite_shader->set_uniform("projection", camera.get_projection());

        auto group =
            active_scene->get_entity_manager().get_registry().group<LightComponent>(entt::get<TransformComponent>);
        for (auto entity : group) {
            auto [transform_component, lightComponent] = group.get<TransformComponent, LightComponent>(entity);

            Texture* light_sprite = nullptr;
            switch (lightComponent.type) {
                using enum LightType;

            case Directional:
                light_sprite = directional_light_texture;
                break;
            case Point:
                light_sprite = point_light_texture;
                break;
            case Spot:
                light_sprite = spot_light_texture;
                break;
            case LightType::COUNT:
                break;
            }

            Render::get()->get_renderer()->queue_quad(transform_component.transform(), *light_sprite);
        }
        Render::get()->get_renderer()->flush_quads(camera, *unlit_sprite_shader);

        // Reset State
        gl_cache->set_depth_test(true);
    }
    // ARC_POP_RENDER_TAG();

    return output;
}
}