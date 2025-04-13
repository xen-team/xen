#include "deferred_light_pass.hpp"

#include <system/windows.hpp>
#include <render/shader.hpp>
#include <render/texture/cubemap.hpp>
#include <render/render.hpp>
#include <render/renderer/gl_cache.hpp>
#include <render/renderer/renderpass/deferred/deferred_geometry_pass.hpp>
#include <scene/scene.hpp>
#include <scene/components/camera.hpp>
#include <utils/shader_loader.hpp>

namespace xen {
DeferredLightingPass::DeferredLightingPass(Scene& scene) :
    RenderPass(scene), lighting_shader(ShaderLoader::load_shader("deferred/pbr_lighting_pass.glsl"))
{

    framebuffer = std::make_shared<Framebuffer>(Windows::get()->get_main_window()->get_size(), false);
    framebuffer->add_color_texture(ColorAttachmentFormat::FloatingPoint16)
        .add_depth_stencil_texture(DepthStencilAttachmentFormat::NormalizedDepthStencil)
        .create_framebuffer();
}

DeferredLightingPass::DeferredLightingPass(Scene& scene, Framebuffer& custom_framebuffer) :
    RenderPass(scene), framebuffer(std::make_shared<Framebuffer>(custom_framebuffer)),
    lighting_shader(ShaderLoader::load_shader("deferred/pbr_lighting_pass.glsl"))
{
}

LightingPassOutput DeferredLightingPass::execute_lighting_pass(
    ShadowmapPassOutput& shadowmap_data, GBuffer& gbuffer, PreLightingPassOutput& pre_lighting_output,
    CameraComponent& camera, bool use_ibl
)
{
    // Framebuffer setup
    GL_CALL(
        glViewport(0, 0, static_cast<GLint>(framebuffer->get_width()), static_cast<GLint>(framebuffer->get_height()))
    );
    GL_CALL(
        glViewport(0, 0, static_cast<GLint>(framebuffer->get_width()), static_cast<GLint>(framebuffer->get_height()))
    );

    framebuffer->bind();
    framebuffer->clear_all();
    gl_cache->set_depth_test(false);
    gl_cache->set_multisample(false);

    // Move the depth + stencil of the GBuffer to our framebuffer
    // NOTE: Framebuffers have to have identical depth + stencil formats for this to work
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer.get_framebuffer()));
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer->get_framebuffer()));
    GL_CALL(glBlitFramebuffer(
        0, 0, static_cast<GLint>(gbuffer.get_width()), static_cast<GLint>(gbuffer.get_height()), 0, 0,
        static_cast<GLint>(framebuffer->get_width()), static_cast<GLint>(framebuffer->get_height()),
        GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST
    ));

    // Setup initial stencil state
    gl_cache->set_stencil_test(true);
    gl_cache->set_stencil_write_mask(0x00); // Do not update stencil values

    LightSystem& light_system = *active_scene->get_light_system();
    ProbeManager& probe_manager = active_scene->get_probe_manager();

    gl_cache->set_shader(*lighting_shader);
    light_system.bind_lightning_uniforms(*lighting_shader);
    lighting_shader->set_uniform("viewPos", camera.get_position());
    lighting_shader->set_uniform("viewInverse", camera.get_view().inverse());
    lighting_shader->set_uniform("projectionInverse", camera.get_projection().inverse());

    // Bind GBuffer data
    gbuffer.get_albedo()->bind(6);
    lighting_shader->set_uniform("albedoTexture", 6);

    gbuffer.get_normal()->bind(7);
    lighting_shader->set_uniform("normalTexture", 7);

    gbuffer.get_material_info()->bind(8);
    lighting_shader->set_uniform("materialInfoTexture", 8);

    pre_lighting_output.ssao_texture->bind(9);
    lighting_shader->set_uniform("ssaoTexture", 9);

    gbuffer.get_depth_stencil_texture().bind(10);
    lighting_shader->set_uniform("depthTexture", 10);

    // Shadowmap code
    bind_shadowmap(*lighting_shader, shadowmap_data);

    // Finally perform the lighting using the GBuffer

    // IBL Bindings
    Vector3f const& camera_position = camera.get_position();
    probe_manager.bind_probes(camera_position, *lighting_shader); // TODO: Should use camera component

    // Perform lighting on the terrain (turn IBL off)
    // ARC_PUSH_RENDER_TAG("Terrain");
    lighting_shader->set_uniform("computeIBL", 0);
    gl_cache->set_stencil_func(GL_EQUAL, StencilValue::TerrainStencilValue, 0xFF);
    Render::get()->get_renderer()->draw_ndc_plane();
    // ARC_POP_RENDER_TAG();

    // Perform lighting on the models in the scene
    // ARC_PUSH_RENDER_TAG("Opaque Models");
    if (use_ibl) {
        lighting_shader->set_uniform("computeIBL", 1);
    }
    else {
        lighting_shader->set_uniform("computeIBL", 0);
    }
    gl_cache->set_stencil_func(GL_EQUAL, StencilValue::ModelStencilValue, 0xFF);
    Render::get()->get_renderer()->draw_ndc_plane();
    // ARC_POP_RENDER_TAG();

    // Reset state
    gl_cache->set_depth_test(true);
    gl_cache->set_stencil_test(false);

    // Render pass output
    LightingPassOutput pass_output;
    pass_output.output_framebuffer = framebuffer.get();
    return pass_output;
}

void DeferredLightingPass::bind_shadowmap(Shader& shader, ShadowmapPassOutput& shadowmap_data)
{
    LightSystem& light_system = *active_scene->get_light_system();

    bool const has_dir_shadow_map = shadowmap_data.directional_shadowmap_framebuffer != nullptr;
    bool const has_spot_shadow_map = shadowmap_data.spot_light_shadowmap_framebuffer != nullptr;

    shader.set_uniform(
        "dirLightShadowData.lightShadowIndex",
        has_dir_shadow_map ? light_system.get_directional_light_shadow_caster_index() : -1
    );
    shader.set_uniform(
        "spotLightShadowData.lightShadowIndex",
        has_spot_shadow_map ? light_system.get_spot_light_shadow_caster_index() : -1
    );
    shader.set_uniform(
        "pointLightShadowData.lightShadowIndex",
        shadowmap_data.has_point_light_shadows ? light_system.get_point_light_shadow_caster_index() : -1
    );

    if (has_dir_shadow_map) {
        shadowmap_data.directional_shadowmap_framebuffer->get_depth_stencil_texture().bind(0);
        shader.set_uniform("dirLightShadowmap", 0);
        shader.set_uniform(
            "dirLightShadowData.lightSpaceViewProjectionMatrix", shadowmap_data.directional_light_view_proj
        );
        shader.set_uniform("dirLightShadowData.shadowBias", shadowmap_data.directional_shadowmap_bias);
    }
    if (has_spot_shadow_map) {
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