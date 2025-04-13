#include "water_pass.hpp"

#include <scene/scene.hpp>
#include <render/shader.hpp>
#include <render/texture/texture.hpp>
#include <render/water/water_system.hpp>
#include <render/renderer/gl_cache.hpp>
#include <render/render.hpp>
#include <render/renderer/renderpass/shadowmap_pass.hpp>
#include <render/renderer/renderpass/forward/forward_light_pass.hpp>
#include <scene/component.hpp>
#include <scene/components/camera.hpp>
#include <scene/components/transform.hpp>
#include <scene/components/water.hpp>
#include <utils/shader_loader.hpp>
#include <resource/resources.hpp>

namespace xen {
WaterPass::WaterPass(Scene& scene) : RenderPass(scene), water_shader(ShaderLoader::load_shader("water.glsl")) {}

WaterPassOutput
WaterPass::execute(ShadowmapPassOutput& shadowmap_data, Framebuffer& framebuffer, CameraComponent& camera)
{
    WaterPassOutput pass_output;
    if (!water_enabled) {
        pass_output.output_framebuffer = &framebuffer;
        return pass_output;
    }

    LightSystem& light_system = *active_scene->get_light_system();
    WaterSystem& water_manager = *active_scene->get_water_system();
    WaterComponent const* closest_water_with_reflection_refraction = water_manager.get_closest_water_component();

    auto group = active_scene->get_entity_manager().get_registry().view<TransformComponent, WaterComponent>();
    for (auto entity : group) {
        auto [transform_component, water_component] = group.get<TransformComponent, WaterComponent>(entity);

        // Setup state
        gl_cache->set_uses_clip_plane(true);

        // Only the closest water can receive reflection and refraction from the environment with planar
        // reflections/refractions
        bool can_reflect_refract = false;
        Framebuffer* reflection_framebuffer = nullptr;
        Framebuffer* refraction_framebuffer = nullptr;
        if (closest_water_with_reflection_refraction == &water_component) {
            can_reflect_refract = true;
        }

        if (can_reflect_refract) {
            reflection_framebuffer = water_manager.get_water_reflection_framebuffer();
            refraction_framebuffer = water_manager.get_water_refraction_framebuffer();

            // Generate Reflection framebuffer and render to it
            // ARC_PUSH_RENDER_TAG("Reflection");
            if (closest_water_with_reflection_refraction->reflection_enabled) {
                Vector2f reflection_near_far_plane = water_manager.get_closest_water_reflection_near_far_plane();
                float prev_near_plane = camera.get_near_plane();
                float prev_far_plane = camera.get_far_plane();

                gl_cache->set_clip_plane(
                    Vector4f(0.0f, 1.0f, 0.0f, -transform_component.position.y + water_component.reflection_plane_bias)
                );
                float distance = 2 * (camera.get_position().y - transform_component.position.y);
                camera.set_position(camera.get_position() - Vector3f(0.0f, distance, 0.0f));
                camera.set_near_plane(reflection_near_far_plane.x);
                camera.set_far_plane(reflection_near_far_plane.x);
                // camera->InvertPitch();

                ForwardLightingPass lighting_pass(*active_scene, *reflection_framebuffer);
                LightingPassOutput output =
                    lighting_pass.execute_opaque_lightning_pass(shadowmap_data, camera, false, false);
                lighting_pass.execute_transparent_lightning_pass(
                    shadowmap_data, *output.output_framebuffer, camera, false, false
                );

                // Check if reflection uses MSAA and resolve it if so
                if (closest_water_with_reflection_refraction->reflection_msaa) {
                    Framebuffer* reflection_resolve_framebuffer =
                        water_manager.get_water_reflection_resolve_framebuffer();

                    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, reflection_framebuffer->get_framebuffer()));
                    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, reflection_resolve_framebuffer->get_framebuffer()));
                    GL_CALL(glBlitFramebuffer(
                        0, 0, static_cast<GLsizei>(reflection_framebuffer->get_width()),
                        static_cast<GLsizei>(reflection_framebuffer->get_height()), 0, 0,
                        static_cast<GLsizei>(reflection_resolve_framebuffer->get_width()),
                        static_cast<GLsizei>(reflection_resolve_framebuffer->get_height()), GL_COLOR_BUFFER_BIT,
                        GL_NEAREST
                    ));

                    reflection_framebuffer =
                        reflection_resolve_framebuffer; // Update reflection framebuffer to the resolved with no MSAA
                }

                camera.set_position(camera.get_position() + Vector3f(0.0f, distance, 0.0f));
                camera.set_near_plane(prev_near_plane);
                camera.set_far_plane(prev_far_plane);
                // camera->InvertPitch();
            }
            // ARC_POP_RENDER_TAG();

            // Generate Refraction framebuffer and render to it
            // ARC_PUSH_RENDER_TAG("Refraction");
            if (closest_water_with_reflection_refraction->refraction_enabled) {
                Vector2f refraction_near_far_plane = water_manager.get_closest_water_refraction_near_far_plane();
                float const prev_near_plane = camera.get_near_plane();
                float const prev_far_plane = camera.get_far_plane();

                gl_cache->set_clip_plane(
                    Vector4f(0.0f, -1.0f, 0.0f, transform_component.position.y + water_component.refraction_plane_bias)
                );
                camera.set_near_plane(refraction_near_far_plane.x);
                camera.set_near_plane(refraction_near_far_plane.y);

                ForwardLightingPass lighting_pass(*active_scene, *refraction_framebuffer);
                LightingPassOutput output =
                    lighting_pass.execute_opaque_lightning_pass(shadowmap_data, camera, false, false);
                lighting_pass.execute_transparent_lightning_pass(
                    shadowmap_data, *output.output_framebuffer, camera, false, false
                );

                // Check if refraction uses MSAA and resolve it if so
                if (closest_water_with_reflection_refraction->refraction_msaa) {
                    Framebuffer* refraction_resolve_framebuffer =
                        water_manager.get_water_refraction_resolve_framebuffer();

                    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, refraction_framebuffer->get_framebuffer()));
                    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, refraction_resolve_framebuffer->get_framebuffer()));
                    GL_CALL(glBlitFramebuffer(
                        0, 0, static_cast<GLsizei>(refraction_framebuffer->get_width()),
                        static_cast<GLsizei>(refraction_framebuffer->get_height()), 0, 0,
                        static_cast<GLsizei>(refraction_resolve_framebuffer->get_width()),
                        static_cast<GLsizei>(refraction_resolve_framebuffer->get_height()),
                        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST
                    ));

                    refraction_framebuffer =
                        refraction_resolve_framebuffer; // Update refraction framebuffer to the resolved with no MSAA
                }

                camera.set_near_plane(prev_near_plane);
                camera.set_far_plane(prev_far_plane);
            }
            // ARC_POP_RENDER_TAG();
        }

        // Reset State
        gl_cache->set_uses_clip_plane(false);

        // Finally render the water geometry and shade it
        // ARC_PUSH_RENDER_TAG("Water");
        gl_cache->set_shader(*water_shader);
        framebuffer.bind();

        GL_CALL(glViewport(
            0, 0, static_cast<GLsizei>(framebuffer.get_width()), static_cast<GLsizei>(framebuffer.get_height())
        ));

        gl_cache->set_multisample(framebuffer.is_multisampled());
        gl_cache->set_depth_test(true);
        gl_cache->set_blend(false);
        gl_cache->set_face_cull(true);
        gl_cache->set_cull_face(GL_BACK);

        Matrix4 const translate = Matrix4(1.f).translate(transform_component.position);
        Matrix4 const rotate = Matrix4(1.f).rotate(-90.0f, Vector3f(1.0f, 0.0f, 0.0f));
        Matrix4 const scale = Matrix4(1.f).scale(transform_component.scale);
        Matrix4 const model = translate * rotate * scale;

        water_component.move_timer = static_cast<float>(effects_timer.elapsed() * water_component.wave_speed);
        water_component.move_timer = std::fmod(water_component.move_timer, 1.f);

        light_system.bind_lightning_uniforms(*water_shader);
        water_shader->set_uniform("view", camera.get_view());
        water_shader->set_uniform("projection", camera.get_projection());
        water_shader->set_uniform("viewInverse", camera.get_view().inverse());
        water_shader->set_uniform("projectionInverse", camera.get_projection().inverse());
        water_shader->set_uniform("clearWater", water_component.clear_water);
        water_shader->set_uniform("shouldShine", water_component.enable_shine);
        water_shader->set_uniform("viewPos", camera.get_position());
        water_shader->set_uniform("waterAlbedo", water_component.water_albedo);
        water_shader->set_uniform("albedoPower", water_component.albedo_power);
        water_shader->set_uniform("model", model);
        water_shader->set_uniform("waveTiling", water_component.water_tiling);
        water_shader->set_uniform("waveMoveFactor", water_component.move_timer);
        water_shader->set_uniform("waveStrength", water_component.wave_strength);
        water_shader->set_uniform("shineDamper", water_component.shine_damper);
        water_shader->set_uniform("waterNormalSmoothing", water_component.normal_smoothing);
        water_shader->set_uniform("depthDampeningEffect", water_component.depth_dampening);

        // Only setup reflections/refractions if it is the closest water component received by the WaterSystem, and it
        // has those options enabled
        bool const reflection = can_reflect_refract && water_component.reflection_enabled;
        bool const refraction = can_reflect_refract && water_component.refraction_enabled;
        water_shader->set_uniform("reflectionEnabled", reflection);
        if (reflection) {
            water_shader->set_uniform("reflectionTexture", 0);
            reflection_framebuffer->get_color_texture().bind(0);
        }
        water_shader->set_uniform("refractionEnabled", refraction);
        if (refraction) {
            water_shader->set_uniform("refractionTexture", 1);
            refraction_framebuffer->get_color_texture().bind(1);
            water_shader->set_uniform("refractionDepthTexture", 4);
            refraction_framebuffer->get_depth_stencil_texture().bind(4);
        }

        water_shader->set_uniform("dudvWaveTexture", 2);
        if (water_component.water_distortion_texture) {
            water_component.water_distortion_texture->bind(2);
        }
        else {
            xen::Resources::get_default_water_distortion_texture()->bind(2);
        }

        water_shader->set_uniform("normalMap", 3);
        if (water_component.water_normal_map) {
            water_component.water_normal_map->bind(3);
        }
        else {
            xen::Resources::get_default_normal_texture()->bind(3);
        }

        water_plane.draw();
        // ARC_POP_RENDER_TAG();
    }

    pass_output.output_framebuffer = &framebuffer;
    return pass_output;
}
}