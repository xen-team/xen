#include "forward_probe_pass.hpp"

#include <scene/scenes.hpp>
#include <render/ibl/probe_manager.hpp>
#include <render/ibl/light_probe.hpp>
#include <render/ibl/reflection_probe.hpp>
#include <render/shader.hpp>
#include <render/skybox.hpp>
#include <render/render.hpp>
#include <render/renderer/renderpass/forward/forward_light_pass.hpp>
#include <render/renderer/renderpass/shadowmap_pass.hpp>
#include <render/renderer/gl_cache.hpp>
#include <utils/shader_loader.hpp>

namespace xen {
ForwardProbePass::ForwardProbePass(Scene& scene) :
    RenderPass(scene),
    scene_capture_dir_light_shadow_framebuffer(Vector2ui{ibl_capture_resolution, ibl_capture_resolution}, false),
    scene_capture_spot_lights_shadow_framebuffer(Vector2ui{ibl_capture_resolution, ibl_capture_resolution}, false),
    scene_capture_lighting_framebuffer(Vector2ui{ibl_capture_resolution, ibl_capture_resolution}, false),
    light_probe_convolution_framebuffer(Vector2ui{light_probe_resolution, light_probe_resolution}, false),
    reflection_probe_sampling_framebuffer(Vector2ui{reflection_probe_resolution, reflection_probe_resolution}, false),
    convolution_shader(ShaderLoader::load_shader("light_probe_convolution.glsl")),
    importance_sampling_shader(ShaderLoader::load_shader("reflection_probe_importance_sampling.glsl"))
{
    scene_capture_settings.texture_format = GL_RGBA16F;
    scene_capture_cubemap.set_cubemap_settings(scene_capture_settings);

    CubemapSettings depth_cubemap_settings;
    depth_cubemap_settings.texture_format = GL_DEPTH_COMPONENT;
    depth_cubemap_settings.texture_minification_filter_mode = GL_LINEAR;
    depth_cubemap_settings.texture_magnification_filter_mode = GL_LINEAR;
    scene_capture_point_light_depth_cubemap.set_cubemap_settings(depth_cubemap_settings);

    scene_capture_dir_light_shadow_framebuffer
        .add_depth_stencil_texture(DepthStencilAttachmentFormat::NormalizedDepthOnly, true)
        .create_framebuffer();
    scene_capture_spot_lights_shadow_framebuffer
        .add_depth_stencil_texture(DepthStencilAttachmentFormat::NormalizedDepthOnly, true)
        .create_framebuffer();
    scene_capture_lighting_framebuffer.add_color_texture(ColorAttachmentFormat::FloatingPoint16)
        .add_depth_stencil_rbo(DepthStencilAttachmentFormat::NormalizedDepthOnly)
        .create_framebuffer();
    light_probe_convolution_framebuffer.add_color_texture(ColorAttachmentFormat::FloatingPoint16).create_framebuffer();
    reflection_probe_sampling_framebuffer.add_color_texture(ColorAttachmentFormat::FloatingPoint16)
        .create_framebuffer();

    scene_capture_point_light_depth_cubemap.generate_cubemap_faces(
        6, GL_TEXTURE_CUBE_MAP_POSITIVE_X, Vector2ui{ibl_capture_resolution, ibl_capture_resolution},
        GL_DEPTH_COMPONENT, nullptr
    );

    scene_capture_cubemap.generate_cubemap_faces(
        6, GL_TEXTURE_CUBE_MAP_POSITIVE_X, Vector2ui{ibl_capture_resolution, ibl_capture_resolution}, GL_RGB, nullptr
    );
}

void ForwardProbePass::pregenerate_ibl()
{
    generate_brdflut();
    generate_fallback_probes();
}

void ForwardProbePass::pregenerate_probes()
{
    // Temp for now, just generate a probe so we have something
    Vector3f probe_position = Vector3f(-32.60f, 10.0f, 48.48f);
    generate_light_probe(probe_position);
    generate_reflection_probe(probe_position);
}

void ForwardProbePass::generate_brdflut()
{
    std::shared_ptr<Shader> brdf_integration_shader = ShaderLoader::load_shader("brdf_integration.glsl");

    // Texture settings for the BRDF LUT
    TextureSettings texture_settings;
    texture_settings.texture_format = GL_RG16F;
    texture_settings.is_srgb = false;
    texture_settings.texture_wrap_s_mode = GL_CLAMP_TO_EDGE;
    texture_settings.texture_wrap_t_mode = GL_CLAMP_TO_EDGE;
    texture_settings.texture_minification_filter_mode = GL_LINEAR;
    texture_settings.texture_magnification_filter_mode = GL_LINEAR;
    texture_settings.texture_anisotropy_level = 1.0f;
    texture_settings.has_mips = false;

    auto brdf_lut = std::make_unique<Texture>(texture_settings);
    brdf_lut->generate_2d_texture(Vector2ui{brdf_lut_resolution, brdf_lut_resolution}, GL_RGB);

    // Setup the framebuffer that we are using to generate our BRDF LUT
    Framebuffer brdf_framebuffer(Vector2ui{brdf_lut_resolution, brdf_lut_resolution}, false);
    brdf_framebuffer.add_color_texture(ColorAttachmentFormat::Normalized8).create_framebuffer();
    brdf_framebuffer.bind();

    // Render state
    gl_cache->set_shader(*brdf_integration_shader);
    gl_cache->set_depth_test(false); // Important cause the depth buffer isn't cleared so it has zero depth

    // Render an NDC quad to the screen so we can generate the BRDF LUT
    glViewport(0, 0, brdf_lut_resolution, brdf_lut_resolution);
    brdf_framebuffer.set_color_attachment(brdf_lut->get_id(), GL_TEXTURE_2D);
    Render::get()->get_renderer()->draw_ndc_plane();
    brdf_framebuffer.set_color_attachment(0, GL_TEXTURE_2D);

    gl_cache->set_depth_test(true);

    // Set the BRDF LUT for all reflection probes
    ReflectionProbe::set_brdf_lut(std::move(brdf_lut));
}

void ForwardProbePass::generate_fallback_probes()
{
    ProbeManager& probe_manager = active_scene->get_probe_manager();
    Vector3f origin(0.0f, 0.0f, 0.0f);
    camera.set_position(origin);

    // Light probe generation
    auto fallback_light_probe =
        std::make_unique<LightProbe>(origin, Vector2f(light_probe_resolution, light_probe_resolution));
    fallback_light_probe->generate();

    gl_cache->set_shader(*convolution_shader);
    gl_cache->set_face_cull(false);
    gl_cache->set_depth_test(false); // Important cause the depth buffer isn't cleared so it has zero depth

    convolution_shader->set_uniform("projection", camera.get_projection());
    active_scene->get_skybox().get_skybox_cubemap()->bind(0);
    convolution_shader->set_uniform("sceneCaptureCubemap", 0);

    light_probe_convolution_framebuffer.bind();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(light_probe_convolution_framebuffer.get_width()),
        static_cast<GLsizei>(light_probe_convolution_framebuffer.get_height())
    ));

    for (int i = 0; i < 6; i++) {
        // Setup the camera's view
        camera.correct_camera_to_cubemap_face(i);
        convolution_shader->set_uniform("view", camera.get_view());

        // Convolute the scene's capture and store it in the Light Probe's cubemap
        light_probe_convolution_framebuffer.set_color_attachment(
            fallback_light_probe->get_irradiance_map()->get_id(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
        );
        Render::get()->get_renderer()->draw_ndc_cube(); // Since we are sampling a cubemap, just use a cube
    }
    light_probe_convolution_framebuffer.set_color_attachment(0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    gl_cache->set_face_cull(true);
    gl_cache->set_depth_test(true);

    // Reflection probe generation
    auto fallback_reflection_probe =
        std::make_unique<ReflectionProbe>(origin, Vector2f(reflection_probe_resolution, reflection_probe_resolution));
    fallback_reflection_probe->generate();

    // Take the capture and perform importance sampling on the cubemap's mips that represent increased roughness levels
    gl_cache->set_shader(*importance_sampling_shader);
    gl_cache->set_face_cull(false);
    gl_cache->set_depth_test(false); // Important cause the depth buffer isn't cleared so it has zero depth

    importance_sampling_shader->set_uniform("projection", camera.get_projection());
    active_scene->get_skybox().get_skybox_cubemap()->bind(0);
    importance_sampling_shader->set_uniform("sceneCaptureCubemap", 0);

    reflection_probe_sampling_framebuffer.bind();
    for (int mip = 0; mip < reflection_probe_mip_count; mip++) {
        // Calculate the size of this mip and resize
        uint const mip_width = reflection_probe_sampling_framebuffer.get_width() >> mip;
        uint const mip_height = reflection_probe_sampling_framebuffer.get_height() >> mip;

        GL_CALL(glViewport(0, 0, static_cast<GLsizei>(mip_width), static_cast<GLsizei>(mip_height)));

        float const mip_roughness_level = static_cast<float>(mip) / (reflection_probe_mip_count - 1);
        importance_sampling_shader->set_uniform("roughness", mip_roughness_level);
        for (int i = 0; i < 6; i++) {
            // Setup the camera's view
            camera.correct_camera_to_cubemap_face(i);
            importance_sampling_shader->set_uniform("view", camera.get_view());

            // Importance sample the scene's capture and store it in the Reflection Probe's cubemap
            reflection_probe_sampling_framebuffer.set_color_attachment(
                fallback_reflection_probe->get_prefilter_map()->get_id(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip
            );
            Render::get()->get_renderer()->draw_ndc_cube(); // Since we are sampling a cubemap, just use a cube
        }
        reflection_probe_sampling_framebuffer.set_color_attachment(0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    }
    gl_cache->set_face_cull(true);
    gl_cache->set_depth_test(true);

    probe_manager.set_light_probe_fallback(std::move(fallback_light_probe));
    probe_manager.set_reflection_probe_fallback(std::move(fallback_reflection_probe));
}

void ForwardProbePass::generate_light_probe(Vector3f const& probe_position)
{
    auto light_probe =
        std::make_unique<LightProbe>(probe_position, Vector2f(light_probe_resolution, light_probe_resolution));
    light_probe->generate();

    // Initialize step before rendering to the probe's cubemap
    camera.set_position(probe_position);
    ShadowmapPass shadow_pass(
        *active_scene, scene_capture_dir_light_shadow_framebuffer, scene_capture_spot_lights_shadow_framebuffer,
        scene_capture_point_light_depth_cubemap
    );
    ForwardLightingPass lighting_pass(
        *active_scene, scene_capture_lighting_framebuffer
    ); // Use our framebuffer when rendering

    // Render the scene to the probe's cubemap
    for (int i = 0; i < 6; i++) {
        // Setup the camera's view
        camera.correct_camera_to_cubemap_face(i);

        // Shadow pass
        ShadowmapPassOutput shadowpass_output = shadow_pass.generate_shadowmaps(camera, true);

        // Light pass
        scene_capture_lighting_framebuffer.bind();
        scene_capture_lighting_framebuffer.set_color_attachment(
            scene_capture_cubemap.get_id(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
        );

        LightingPassOutput output = lighting_pass.execute_opaque_lightning_pass(shadowpass_output, camera, true, false);
        lighting_pass.execute_transparent_lightning_pass(
            shadowpass_output, *output.output_framebuffer, camera, true, false
        );
    }
    scene_capture_lighting_framebuffer.set_color_attachment(0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);

    // Take the capture and apply convolution for the irradiance map (indirect diffuse)
    gl_cache->set_shader(*convolution_shader);
    gl_cache->set_face_cull(false);
    gl_cache->set_depth_test(false); // Important cause the depth buffer isn't cleared so it has zero depth

    convolution_shader->set_uniform("projection", camera.get_projection());
    scene_capture_cubemap.bind(0);
    convolution_shader->set_uniform("sceneCaptureCubemap", 0);

    light_probe_convolution_framebuffer.bind();

    GL_CALL(glViewport(
        0, 0, static_cast<GLsizei>(light_probe_convolution_framebuffer.get_width()),
        static_cast<GLsizei>(light_probe_convolution_framebuffer.get_height())
    ));

    for (int i = 0; i < 6; i++) {
        // Setup the camera's view
        camera.correct_camera_to_cubemap_face(i);
        convolution_shader->set_uniform("view", camera.get_view());

        // Convolute the scene's capture and store it in the Light Probe's cubemap
        light_probe_convolution_framebuffer.set_color_attachment(
            light_probe->get_irradiance_map()->get_id(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
        );
        Render::get()->get_renderer()->draw_ndc_cube(); // Since we are sampling a cubemap, just use a cube
    }

    light_probe_convolution_framebuffer.set_color_attachment(0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    gl_cache->set_face_cull(true);
    gl_cache->set_depth_test(true);

    ProbeManager& probe_manager = active_scene->get_probe_manager();
    probe_manager.add_probe(std::move(light_probe));
}

void ForwardProbePass::generate_reflection_probe(Vector3f const& probe_position)
{
    auto reflection_probe = std::make_unique<ReflectionProbe>(
        probe_position, Vector2f(reflection_probe_resolution, reflection_probe_resolution)
    );
    reflection_probe->generate();

    // Initialize step before rendering to the probe's cubemap
    camera.set_position(probe_position);
    ShadowmapPass shadow_pass(
        *active_scene, scene_capture_dir_light_shadow_framebuffer, scene_capture_spot_lights_shadow_framebuffer,
        scene_capture_point_light_depth_cubemap
    );
    ForwardLightingPass lighting_pass(
        *active_scene, scene_capture_lighting_framebuffer
    ); // Use our framebuffer when rendering

    // Render the scene to the probe's cubemap
    for (int i = 0; i < 6; i++) {
        // Setup the camera's view
        camera.correct_camera_to_cubemap_face(i);

        // Shadow pass
        ShadowmapPassOutput shadowpass_output = shadow_pass.generate_shadowmaps(camera, true);

        // Light pass
        scene_capture_lighting_framebuffer.bind();
        scene_capture_lighting_framebuffer.set_color_attachment(
            scene_capture_cubemap.get_id(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
        );
        LightingPassOutput output = lighting_pass.execute_opaque_lightning_pass(shadowpass_output, camera, true, false);
        lighting_pass.execute_transparent_lightning_pass(
            shadowpass_output, *output.output_framebuffer, camera, true, false
        );
    }
    scene_capture_lighting_framebuffer.set_color_attachment(0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);

    // Take the capture and perform importance sampling on the cubemap's mips that represent increased roughness levels
    gl_cache->set_shader(*importance_sampling_shader);
    gl_cache->set_face_cull(false);
    gl_cache->set_depth_test(false); // Important cause the depth buffer isn't cleared so it has zero depth

    importance_sampling_shader->set_uniform("projection", camera.get_projection());
    scene_capture_cubemap.bind(0);
    importance_sampling_shader->set_uniform("sceneCaptureCubemap", 0);

    reflection_probe_sampling_framebuffer.bind();
    for (int mip = 0; mip < reflection_probe_mip_count; mip++) {
        // Calculate the size of this mip and resize
        uint const mip_width = reflection_probe_sampling_framebuffer.get_width() >> mip;
        uint const mip_height = reflection_probe_sampling_framebuffer.get_height() >> mip;

        glViewport(0, 0, static_cast<GLsizei>(mip_width), static_cast<GLsizei>(mip_height));

        float const mip_roughness_level = static_cast<float>(mip) / (reflection_probe_mip_count - 1);
        importance_sampling_shader->set_uniform("roughness", mip_roughness_level);
        for (int i = 0; i < 6; i++) {
            // Setup the camera's view
            camera.correct_camera_to_cubemap_face(i);
            importance_sampling_shader->set_uniform("view", camera.get_view());

            // Importance sample the scene's capture and store it in the Reflection Probe's cubemap
            reflection_probe_sampling_framebuffer.set_color_attachment(
                reflection_probe->get_prefilter_map()->get_id(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip
            );
            Render::get()->get_renderer()->draw_ndc_cube(); // Since we are sampling a cubemap, just use a cube
        }
        reflection_probe_sampling_framebuffer.set_color_attachment(0, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    }
    gl_cache->set_face_cull(true);
    gl_cache->set_depth_test(true);

    ProbeManager& probe_manager = active_scene->get_probe_manager();
    probe_manager.add_probe(std::move(reflection_probe));
}
}