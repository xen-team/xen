#include "forward_light_pass.hpp"

#include <system/windows.hpp>
#include <render/shader.hpp>
#include <render/skybox.hpp>
#include <render/texture/cubemap.hpp>
#include <render/render.hpp>
#include <render/renderer/gl_cache.hpp>
#include <render/renderer/renderpass/deferred/deferred_geometry_pass.hpp>
#include <render/terrain/terrain.hpp>
#include <scene/scene.hpp>
#include <scene/components/camera.hpp>
#include <utils/shader_loader.hpp>

namespace xen {
ForwardLightingPass::ForwardLightingPass(Scene& scene, bool should_multisample) : RenderPass(scene)
{
    init();

    framebuffer = std::make_shared<Framebuffer>(Windows::get()->get_main_window()->get_size(), should_multisample);
    framebuffer->add_color_texture(ColorAttachmentFormat::FloatingPoint16)
        .add_depth_stencil_texture(DepthStencilAttachmentFormat::NormalizedDepthStencil)
        .create_framebuffer();
}

ForwardLightingPass::ForwardLightingPass(Scene& scene, Framebuffer& custom_framebuffer) :
    RenderPass(scene), framebuffer(std::make_shared<Framebuffer>(custom_framebuffer))
{
    init();
}

void ForwardLightingPass::init()
{
    model_shader = ShaderLoader::load_shader("forward/pbr_model.glsl");
    skinned_model_shader = ShaderLoader::load_shader("forward/pbr_skinned_model.glsl");
    terrain_shader = ShaderLoader::load_shader("forward/pbr_terrain.glsl");
}

LightingPassOutput ForwardLightingPass::execute_opaque_lightning_pass(
    ShadowmapPassOutput& shadowmap_data, CameraComponent& camera, bool render_only_static, bool use_ibl
)
{
    glViewport(0, 0, static_cast<GLsizei>(framebuffer->get_width()), static_cast<GLsizei>(framebuffer->get_height()));
    framebuffer->bind();
    framebuffer->clear_all();
    if (framebuffer->is_multisampled()) {
        gl_cache->set_multisample(true);
    }
    else {
        gl_cache->set_multisample(false);
    }

    // Setup
    LightSystem& light_system = *active_scene->get_light_system();
    ProbeManager& probe_manager = active_scene->get_probe_manager();

    // Lighting setup
    auto lightbind_function = &LightSystem::bind_lightning_uniforms;
    if (render_only_static) {
        lightbind_function = &LightSystem::bind_static_lighting_uniforms;
    }

    // Render terrain
    Terrain& terrain = active_scene->get_terrain();

    // ARC_PUSH_RENDER_TAG("Terrain");
    gl_cache->set_shader(*terrain_shader);
    if (gl_cache->get_uses_clip_plane()) {
        terrain_shader->set_uniform("usesClipPlane", true);
        terrain_shader->set_uniform("clipPlane", gl_cache->get_active_clip_plane());
    }
    else {
        terrain_shader->set_uniform("usesClipPlane", false);
    }
    (light_system.*lightbind_function)(*terrain_shader);
    terrain_shader->set_uniform("viewPos", camera.get_position());
    terrain_shader->set_uniform("view", camera.get_view());
    terrain_shader->set_uniform("projection", camera.get_projection());
    bind_shadowmap(*terrain_shader, shadowmap_data);
    terrain.draw(*terrain_shader, RenderPassType::MaterialRequired);
    // ARC_POP_RENDER_TAG();

    // Render opaque objects since we are in the opaque pass
    // Add meshes to the renderer
    if (render_only_static) {
        active_scene->add_models_to_renderer(ModelFilterType::OpaqueStaticModels);
    }
    else {
        active_scene->add_models_to_renderer(ModelFilterType::OpaqueModels);
    }

    // bind data to skinned shader and render skinned models
    // ARC_PUSH_RENDER_TAG("Skinned Models");
    {
        gl_cache->set_shader(*skinned_model_shader);
        if (gl_cache->get_uses_clip_plane()) {
            skinned_model_shader->set_uniform("usesClipPlane", true);
            skinned_model_shader->set_uniform("clipPlane", gl_cache->get_active_clip_plane());
        }
        else {
            skinned_model_shader->set_uniform("usesClipPlane", false);
        }
        (light_system.*lightbind_function)(*skinned_model_shader);

        // Shadowmap code
        bind_shadowmap(*skinned_model_shader, shadowmap_data);

        // IBL binding
        Vector3f const camera_position = camera.get_position();
        probe_manager.bind_probes(camera_position, *skinned_model_shader); // TODO: Should use camera component
        if (use_ibl) {
            skinned_model_shader->set_uniform("computeIBL", 1);
        }
        else {
            skinned_model_shader->set_uniform("computeIBL", 0);
        }

        Render::get()->get_renderer()->flush_opaque_skinned_meshes(
            camera, RenderPassType::MaterialRequired, *skinned_model_shader
        );
    }
    // ARC_POP_RENDER_TAG();

    // bind data to non-skinned shader and render non-skinned models
    // ARC_PUSH_RENDER_TAG("Non-Skinned Models");
    {
        gl_cache->set_shader(*model_shader);
        if (gl_cache->get_uses_clip_plane()) {
            model_shader->set_uniform("usesClipPlane", true);
            model_shader->set_uniform("clipPlane", gl_cache->get_active_clip_plane());
        }
        else {
            model_shader->set_uniform("usesClipPlane", false);
        }
        (light_system.*lightbind_function)(*model_shader);

        // Shadowmap code
        bind_shadowmap(*model_shader, shadowmap_data);

        // IBL binding
        Vector3f const camera_position = camera.get_position();
        probe_manager.bind_probes(camera_position, *model_shader); // TODO: Should use camera component
        if (use_ibl) {
            model_shader->set_uniform("computeIBL", 1);
        }
        else {
            model_shader->set_uniform("computeIBL", 0);
        }

        Render::get()->get_renderer()->flush_opaque_non_skinned_meshes(
            camera, RenderPassType::MaterialRequired, *model_shader
        );
    }
    // ARC_POP_RENDER_TAG();

    // Render pass output
    LightingPassOutput pass_output;
    pass_output.output_framebuffer = framebuffer.get();
    return pass_output;
}

LightingPassOutput ForwardLightingPass::execute_transparent_lightning_pass(
    ShadowmapPassOutput& shadowmap_data, Framebuffer& framebuffer, CameraComponent& camera, bool render_only_static,
    bool use_ibl
)
{
    glViewport(0, 0, static_cast<GLsizei>(framebuffer.get_width()), static_cast<GLsizei>(framebuffer.get_height()));
    framebuffer.bind();
    if (framebuffer.is_multisampled()) {
        gl_cache->set_multisample(true);
    }
    else {
        gl_cache->set_multisample(false);
    }
    gl_cache->set_depth_test(true);

    // Setup
    LightSystem& light_system = *active_scene->get_light_system();
    Skybox& skybox = active_scene->get_skybox();
    ProbeManager& probe_manager = active_scene->get_probe_manager();

    // Render skybox
    // ARC_PUSH_RENDER_TAG("Sky");
    skybox.draw(camera);
    // ARC_POP_RENDER_TAG();

    // Lighting setup
    auto lightbind_function = &LightSystem::bind_lightning_uniforms;
    if (render_only_static) {
        lightbind_function = &LightSystem::bind_static_lighting_uniforms;
    }

    // Render transparent objects since we are in the transparent pass
    // Add meshes to the renderer
    if (render_only_static) {
        active_scene->add_models_to_renderer(ModelFilterType::TransparentStaticModels);
    }
    else {
        active_scene->add_models_to_renderer(ModelFilterType::TransparentModels);
    }

    // bind data to skinned shader and render skinned models
    // ARC_PUSH_RENDER_TAG("Skinned Models");
    {
        gl_cache->set_shader(*skinned_model_shader);
        if (gl_cache->get_uses_clip_plane()) {
            skinned_model_shader->set_uniform("usesClipPlane", true);
            skinned_model_shader->set_uniform("clipPlane", gl_cache->get_active_clip_plane());
        }
        else {
            skinned_model_shader->set_uniform("usesClipPlane", false);
        }
        (light_system.*lightbind_function)(*skinned_model_shader);

        // Shadowmap code
        bind_shadowmap(*skinned_model_shader, shadowmap_data);

        // IBL binding
        Vector3f const camera_position = camera.get_position();
        probe_manager.bind_probes(camera_position, *skinned_model_shader); // TODO: Should use camera component
        if (use_ibl) {
            skinned_model_shader->set_uniform("computeIBL", 1);
        }
        else {
            skinned_model_shader->set_uniform("computeIBL", 0);
        }

        Render::get()->get_renderer()->flush_transparent_skinned_meshes(
            camera, RenderPassType::MaterialRequired, *skinned_model_shader
        );
    }
    // ARC_POP_RENDER_TAG();

    // bind data to non-skinned shader and render non-skinned models
    // ARC_PUSH_RENDER_TAG("Non-Skinned Models");
    {
        gl_cache->set_shader(*model_shader);
        if (gl_cache->get_uses_clip_plane()) {
            model_shader->set_uniform("usesClipPlane", true);
            model_shader->set_uniform("clipPlane", gl_cache->get_active_clip_plane());
        }
        else {
            model_shader->set_uniform("usesClipPlane", false);
        }
        (light_system.*lightbind_function)(*model_shader);

        // Shadowmap code
        bind_shadowmap(*model_shader, shadowmap_data);

        // IBL binding
        Vector3f const camera_position = camera.get_position();
        probe_manager.bind_probes(camera_position, *model_shader); // TODO: Should use camera component
        if (use_ibl) {
            model_shader->set_uniform("computeIBL", 1);
        }
        else {
            model_shader->set_uniform("computeIBL", 0);
        }

        Render::get()->get_renderer()->flush_transparent_non_skinned_meshes(
            camera, RenderPassType::MaterialRequired, *model_shader
        );
    }
    // ARC_POP_RENDER_TAG();

    // Render pass output
    LightingPassOutput pass_output;
    pass_output.output_framebuffer = &framebuffer;
    return pass_output;
}

void ForwardLightingPass::bind_shadowmap(Shader& shader, ShadowmapPassOutput& shadowmap_data)
{
    LightSystem& light_system = *active_scene->get_light_system();

    bool hasDirShadowMap = shadowmap_data.directional_shadowmap_framebuffer != nullptr;
    bool hasSpotShadowMap = shadowmap_data.spot_light_shadowmap_framebuffer != nullptr;

    shader.set_uniform(
        "dirLightShadowData.lightShadowIndex",
        hasDirShadowMap ? light_system.get_directional_light_shadow_caster_index() : -1
    );
    shader.set_uniform(
        "spotLightShadowData.lightShadowIndex",
        hasSpotShadowMap ? light_system.get_spot_light_shadow_caster_index() : -1
    );
    shader.set_uniform(
        "pointLightShadowData.lightShadowIndex",
        shadowmap_data.has_point_light_shadows ? light_system.get_point_light_shadow_caster_index() : -1
    );

    if (hasDirShadowMap) {
        shadowmap_data.directional_shadowmap_framebuffer->get_depth_stencil_texture().bind(0);
        shader.set_uniform("dirLightShadowmap", 0);
        shader.set_uniform(
            "dirLightShadowData.lightSpaceViewProjectionMatrix", shadowmap_data.directional_light_view_proj
        );
        shader.set_uniform("dirLightShadowData.shadowBias", shadowmap_data.directional_shadowmap_bias);
    }
    if (hasSpotShadowMap) {
        shadowmap_data.spot_light_shadowmap_framebuffer->get_depth_stencil_texture().bind(1);
        shader.set_uniform("spotLightShadowmap", 1);
        shader.set_uniform("spotLightShadowData.lightSpaceViewProjectionMatrix", shadowmap_data.spot_light_view_proj);
        shader.set_uniform("spotLightShadowData.shadowBias", shadowmap_data.spot_light_shadowmap_bias);
    }
    if (shadowmap_data.has_point_light_shadows) {
        shader.set_uniform("pointLightShadowData.shadowBias", shadowmap_data.point_light_shadowmap_bias);
        shader.set_uniform("pointLightShadowData.farPlane", shadowmap_data.point_light_far_plane);
    }
    shader.set_uniform("pointLightShadowCubemap", 2);
    shadowmap_data.point_light_shadow_cubemap->bind(2
    ); // Must be bound even if there is no point light shadows. Thanks OpenGL Driver!
}
}