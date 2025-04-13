#include "shadowmap_pass.hpp"

#include <scene/scene.hpp>
#include <scene/components/camera.hpp>
#include <render/render.hpp>
#include <render/renderer/gl_cache.hpp>
#include <render/lights/light_system.hpp>
#include <render/shader.hpp>
#include <render/texture/cubemap.hpp>
#include <render/terrain/terrain.hpp>
#include <utils/shader_loader.hpp>

namespace xen {
ShadowmapPass::ShadowmapPass(Scene& scene) : RenderPass(scene)
{
    init();
}

ShadowmapPass::ShadowmapPass(
    Scene& scene, Framebuffer& custom_directional_light_shadow_framebuffer,
    Framebuffer& custom_spot_light_shadow_framebuffer, Cubemap& custom_point_light_shadow_cubemap
) :
    RenderPass(scene), custom_directional_light_shadow_framebuffer(&custom_directional_light_shadow_framebuffer),
    custom_spot_light_shadow_framebuffer(&custom_spot_light_shadow_framebuffer),
    custom_point_light_shadow_cubemap(&custom_point_light_shadow_cubemap)
{
    init();
}

void ShadowmapPass::init()
{
    shadowmap_shader = ShaderLoader::load_shader("shadowmap_generation.glsl");
    shadowmap_skinned_shader = ShaderLoader::load_shader("shadowmap_generation_skinned.glsl");
    shadowmap_linear_shader = ShaderLoader::load_shader("shadowmap_generation_linear.glsl");
    shadowmap_linear_skinned_shader = ShaderLoader::load_shader("shadowmap_generation_linear_skinned.glsl");
    empty_framebuffer.add_depth_stencil_texture(DepthStencilAttachmentFormat::NormalizedDepthOnly, true)
        .create_framebuffer();
}

ShadowmapPassOutput ShadowmapPass::generate_shadowmaps(CameraComponent& camera, bool render_only_static)
{
    ShadowmapPassOutput pass_output;

    LightSystem& light_system = *active_scene->get_light_system();

    Framebuffer* shadow_framebuffer = custom_directional_light_shadow_framebuffer ?
                                          custom_directional_light_shadow_framebuffer :
                                          light_system.get_directional_light_shadow_framebuffer();

    // Directional Light Shadow Setup
    // ARC_PUSH_RENDER_TAG("Directional Shadows");
    glViewport(
        0, 0, static_cast<GLsizei>(shadow_framebuffer->get_width()),
        static_cast<GLsizei>(shadow_framebuffer->get_height())
    );

    shadow_framebuffer->bind();
    shadow_framebuffer->clear_depth();

    // Directional Light Shadows
    if (light_system.has_directional_light_shadow_caster()) {
        // Setup
        Vector2f near_far_plane = light_system.get_directional_light_shadow_caster_near_far_plane();

        // View + Projection setup
        Vector3f const dir_light_shadowmap_look_at_pos = camera.get_position();

        Vector3f const dir_light_shadowmap_eye_pos =
            dir_light_shadowmap_look_at_pos - (light_system.get_directional_light_shadow_caster_light_dir() * 50.0f);

        auto const directional_light_projection =
            Matrix4::orthographic_matrix(-40.0f, 40.0f, -40.0f, 40.0f, near_far_plane.x, near_far_plane.y);

        auto const directional_light_view =
            Matrix4::look_at(dir_light_shadowmap_eye_pos, dir_light_shadowmap_look_at_pos, Vector3f(0.0f, 1.0f, 0.0f));

        Matrix4 const directional_light_view_matrix = directional_light_projection * directional_light_view;

        gl_cache->set_depth_test(true);
        gl_cache->set_blend(false);
        gl_cache->set_face_cull(false
        ); // For one sided objects - TODO: This will get overwritten by the renderer anyways

        // Setup model renderer
        if (render_only_static) {
            active_scene->add_models_to_renderer(ModelFilterType::StaticModels);
        }
        else {
            active_scene->add_models_to_renderer(ModelFilterType::AllModels);
        }

        // Render skinned models
        {
            gl_cache->set_shader(*shadowmap_skinned_shader);
            shadowmap_skinned_shader->set_uniform("lightSpaceViewProjectionMatrix", directional_light_view_matrix);
            Render::get()->get_renderer()->flush_opaque_skinned_meshes(
                camera, RenderPassType::NoMaterialRequired, *shadowmap_skinned_shader
            ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for lights
            Render::get()->get_renderer()->flush_transparent_skinned_meshes(
                camera, RenderPassType::NoMaterialRequired, *shadowmap_skinned_shader
            ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for lights
        }

        // Render non-skinned models
        {
            gl_cache->set_shader(*shadowmap_shader);
            shadowmap_shader->set_uniform("lightSpaceViewProjectionMatrix", directional_light_view_matrix);
            Render::get()->get_renderer()->flush_opaque_non_skinned_meshes(
                camera, RenderPassType::NoMaterialRequired, *shadowmap_shader
            ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for lights
            Render::get()->get_renderer()->flush_transparent_non_skinned_meshes(
                camera, RenderPassType::NoMaterialRequired, *shadowmap_shader
            ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for lights
        }

        // Render terrain
        Terrain& terrain = active_scene->get_terrain();
        terrain.draw(*shadowmap_shader, RenderPassType::NoMaterialRequired);

        // Update output
        pass_output.directional_light_view_proj = directional_light_view_matrix;
        pass_output.directional_shadowmap_bias = light_system.get_directional_light_shadow_caster_bias();
        pass_output.directional_shadowmap_framebuffer = shadow_framebuffer;
    }

    // ARC_POP_RENDER_TAG();

    // Spot Light Shadow Setup
    // ARC_PUSH_RENDER_TAG("Spotlight Shadows");

    shadow_framebuffer =
        (custom_spot_light_shadow_framebuffer ? custom_spot_light_shadow_framebuffer :
                                                light_system.get_spot_light_shadow_caster_framebuffer());

    glViewport(
        0, 0, static_cast<GLsizei>(shadow_framebuffer->get_width()),
        static_cast<GLsizei>(shadow_framebuffer->get_height())
    );

    shadow_framebuffer->bind();
    shadow_framebuffer->clear_depth();

    // Spot Light Shadows
    if (light_system.has_spot_light_shadow_caster()) {
        // Setup
        Vector2f near_far_plane = light_system.get_spot_light_shadow_caster_near_far_plane();

        // View + Projection setup
        float outer_angle_radians = light_system.get_spot_light_shadow_caster_outer_cut_off_angle();
        float radius =
            light_system.get_spot_light_shadow_caster_attenuation_range() *
            std::tan(outer_angle_radians
            ); // Need to get spotlight's radius given it's range and angle so we can use it for the projection bounds
        Matrix4 const spot_light_projection =
            Matrix4::orthographic_matrix(-radius, radius, -radius, radius, near_far_plane.x, near_far_plane.y);
        Vector3f const spot_light_pos = light_system.get_spot_light_shadow_caster_light_position();
        Matrix4 const spot_light_view = Matrix4::look_at(
            spot_light_pos, spot_light_pos + light_system.get_spot_light_shadow_caster_light_dir(),
            Vector3f(0.0f, 1.0f, 0.0f)
        );
        Matrix4 const spot_light_view_proj = spot_light_projection * spot_light_view;

        gl_cache->set_depth_test(true);
        gl_cache->set_blend(false);
        gl_cache->set_face_cull(false
        ); // For one sided objects - TODO: This will get overwritten by the renderer anyways

        // Setup model renderer
        if (render_only_static) {
            active_scene->add_models_to_renderer(ModelFilterType::StaticModels);
        }
        else {
            active_scene->add_models_to_renderer(ModelFilterType::AllModels);
        }

        // Render skinned models
        {
            gl_cache->set_shader(*shadowmap_skinned_shader);
            shadowmap_skinned_shader->set_uniform("lightSpaceViewProjectionMatrix", spot_light_view_proj);
            Render::get()->get_renderer()->flush_opaque_skinned_meshes(
                camera, RenderPassType::NoMaterialRequired, *shadowmap_skinned_shader
            ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for lights
            Render::get()->get_renderer()->flush_transparent_skinned_meshes(
                camera, RenderPassType::NoMaterialRequired, *shadowmap_skinned_shader
            ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for lights
        }

        // Render non-skinned models
        {
            gl_cache->set_shader(*shadowmap_shader);
            shadowmap_shader->set_uniform("lightSpaceViewProjectionMatrix", spot_light_view_proj);
            Render::get()->get_renderer()->flush_opaque_non_skinned_meshes(
                camera, RenderPassType::NoMaterialRequired, *shadowmap_shader
            ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for lights
            Render::get()->get_renderer()->flush_transparent_non_skinned_meshes(
                camera, RenderPassType::NoMaterialRequired, *shadowmap_shader
            ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for lights
        }

        // Render terrain
        Terrain& terrain = active_scene->get_terrain();
        terrain.draw(*shadowmap_shader, RenderPassType::NoMaterialRequired);

        // Update output
        pass_output.spot_light_view_proj = spot_light_view_proj;
        pass_output.spot_light_shadowmap_bias = light_system.get_spot_light_shadow_caster_bias();
        pass_output.spot_light_shadowmap_framebuffer = shadow_framebuffer;
    }
    // ARC_POP_RENDER_TAG();

    // Point Light Shadow Setup
    // ARC_PUSH_RENDER_TAG("Pointlight Shadows");
    Cubemap* point_light_shadow_cubemap = nullptr;
    if (custom_point_light_shadow_cubemap) {
        point_light_shadow_cubemap = custom_point_light_shadow_cubemap;
    }
    else {
        point_light_shadow_cubemap = light_system.get_point_light_shadow_caster_cubemap();
    }
    empty_framebuffer.bind();

    // Point Light Shadows
    pass_output.has_point_light_shadows = false;
    if (light_system.has_pointlight_shadow_caster()) {
        // Setup
        // Terrain& terrain = active_scene->get_terrain();

        Vector2f const near_far_plane = light_system.get_point_light_shadow_caster_near_far_plane();

        // Camera Setup
        camera.set_position(light_system.get_point_light_shadow_caster_light_position());
        camera.set_near_plane(near_far_plane.x);
        camera.set_far_plane(near_far_plane.y);

        Matrix4 const point_light_projection = camera.get_projection();

        gl_cache->set_depth_test(true);
        gl_cache->set_blend(false);
        gl_cache->set_face_cull(false
        ); // For one sided objects - TODO: This will get overwritten by the renderer anyways

        // Render the scene to the probe's cubemap
        glViewport(
            0, 0, static_cast<GLsizei>(point_light_shadow_cubemap->get_face_width()),
            static_cast<GLsizei>(point_light_shadow_cubemap->get_face_height())
        );

        for (int i = 0; i < 6; i++) {
            // Setup the camera's view
            camera.correct_camera_to_cubemap_face(i);
            Matrix4 const point_light_view = camera.get_view();
            Matrix4 const point_light_view_proj = point_light_projection * point_light_view;

            empty_framebuffer.set_depth_attachment(
                DepthStencilAttachmentFormat::NormalizedDepthOnly, point_light_shadow_cubemap->get_id(),
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
            );
            empty_framebuffer.clear_depth();

            // Setup model renderer
            if (render_only_static) {
                active_scene->add_models_to_renderer(ModelFilterType::StaticModels);
            }
            else {
                active_scene->add_models_to_renderer(ModelFilterType::AllModels);
            }

            // Render skinned models
            {
                gl_cache->set_shader(*shadowmap_linear_skinned_shader);
                shadowmap_linear_skinned_shader->set_uniform("lightPos", camera.get_position());
                shadowmap_linear_skinned_shader->set_uniform("lightFarPlane", near_far_plane.y);
                shadowmap_linear_skinned_shader->set_uniform("lightSpaceViewProjectionMatrix", point_light_view_proj);
                Render::get()->get_renderer()->flush_opaque_skinned_meshes(
                    camera, RenderPassType::NoMaterialRequired, *shadowmap_linear_skinned_shader
                ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for
                   // lights
                Render::get()->get_renderer()->flush_transparent_skinned_meshes(
                    camera, RenderPassType::NoMaterialRequired, *shadowmap_linear_skinned_shader
                ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for
                   // lights
            }

            // Render non-skinned models
            {
                gl_cache->set_shader(*shadowmap_linear_shader);
                shadowmap_linear_shader->set_uniform("lightPos", camera.get_position());
                shadowmap_linear_shader->set_uniform("lightFarPlane", near_far_plane.y);
                shadowmap_linear_shader->set_uniform("lightSpaceViewProjectionMatrix", point_light_view_proj);
                Render::get()->get_renderer()->flush_opaque_non_skinned_meshes(
                    camera, RenderPassType::NoMaterialRequired, *shadowmap_linear_shader
                ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for
                   // lights
                Render::get()->get_renderer()->flush_transparent_non_skinned_meshes(
                    camera, RenderPassType::NoMaterialRequired, *shadowmap_linear_shader
                ); // TODO: This should not use the camera's position for sorting we are rendering shadow maps for
                   // lights
            }

            // Render terrain
            Terrain& terrain = active_scene->get_terrain();
            terrain.draw(*shadowmap_linear_shader, RenderPassType::NoMaterialRequired);
        }
        // Reset state
        empty_framebuffer.set_depth_attachment(
            DepthStencilAttachmentFormat::NormalizedDepthOnly, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X
        );

        // Update output
        pass_output.has_point_light_shadows = true;
        pass_output.point_light_shadowmap_bias = light_system.get_point_light_shadow_caster_bias();
        pass_output.point_light_far_plane = near_far_plane.y;
    }
    pass_output.point_light_shadow_cubemap =
        point_light_shadow_cubemap; // Has to be bound even if it isn't used, thanks to OpenGL Driver
    // ARC_POP_RENDER_TAG();

    return pass_output;
}
}