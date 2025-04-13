#include "deferred_geometry_pass.hpp"

#include <system/windows.hpp>
#include <render/shader.hpp>
#include <render/texture/cubemap.hpp>
#include <render/render.hpp>
#include <render/renderer/gl_cache.hpp>
#include <render/renderer/renderpass/deferred/deferred_geometry_pass.hpp>
#include <render/terrain/terrain.hpp>
#include <scene/scene.hpp>
#include <scene/components/camera.hpp>
#include <utils/shader_loader.hpp>
#include <platform/opengl/gbuffer.hpp>

namespace xen {
DeferredGeometryPass::DeferredGeometryPass(Scene& scene) :
    RenderPass(scene), model_shader(ShaderLoader::load_shader("deferred/pbr_model_geometry_pass.glsl")),
    skinned_model_shader(ShaderLoader::load_shader("deferred/pbr_skinned_model_geometry_pass.glsl")),
    terrain_shader(ShaderLoader::load_shader("deferred/pbr_terrain_geometry_pass.glsl"))
{

    gbuffer = std::make_shared<GBuffer>(Windows::get()->get_main_window()->get_size());
}

DeferredGeometryPass::DeferredGeometryPass(Scene& scene, GBuffer& custom_gbuffer) :
    RenderPass(scene), gbuffer(std::make_shared<GBuffer>(custom_gbuffer)),
    model_shader(ShaderLoader::load_shader("deferred/pbr_model_geometry_pass.glsl")),
    terrain_shader(ShaderLoader::load_shader("deferred/pbr_terrain_geometry_pass.glsl"))
{
}

GeometryPassOutput DeferredGeometryPass::execute_geometry_pass(CameraComponent& camera, bool render_only_static)
{
    GL_CALL(glViewport(0, 0, static_cast<GLint>(gbuffer->get_width()), static_cast<GLint>(gbuffer->get_height())));

    gbuffer->bind();
    gbuffer->clear_all();
    gl_cache->set_blend(false);
    gl_cache->set_multisample(false);

    // Setup initial stencil state
    gl_cache->set_stencil_op(GL_KEEP, GL_KEEP, GL_REPLACE);
    gl_cache->set_stencil_write_mask(0x00);
    gl_cache->set_stencil_test(true);

    // Setup model renderer for opaque objects only
    if (render_only_static) {
        active_scene->add_models_to_renderer(ModelFilterType::OpaqueStaticModels);
    }
    else {
        active_scene->add_models_to_renderer(ModelFilterType::OpaqueModels);
    }

    // Render opaque objects (use stencil to denote models for the deferred lighting pass)
    gl_cache->set_stencil_write_mask(0xFF);
    gl_cache->set_stencil_func(GL_ALWAYS, StencilValue::ModelStencilValue, 0xFF);
    // ARC_PUSH_RENDER_TAG("Skinned Models");
    Render::get()->get_renderer()->flush_opaque_skinned_meshes(
        camera, RenderPassType::MaterialRequired, *skinned_model_shader
    );
    // ARC_POP_RENDER_TAG();
    // ARC_PUSH_RENDER_TAG("Non-Skinned Models");
    Render::get()->get_renderer()->flush_opaque_non_skinned_meshes(
        camera, RenderPassType::MaterialRequired, *model_shader
    );
    // ARC_POP_RENDER_TAG();
    gl_cache->set_stencil_write_mask(0x00);

    Terrain& terrain = active_scene->get_terrain();

    // Setup terrain information
    // ARC_PUSH_RENDER_TAG("Terrain");
    gl_cache->set_shader(*terrain_shader);
    terrain_shader->set_uniform("view", camera.get_view());
    terrain_shader->set_uniform("projection", camera.get_projection());

    // Render the terrain (use stencil to denote the terrain for the deferred lighting pass)
    gl_cache->set_stencil_write_mask(0xFF);
    gl_cache->set_stencil_func(GL_ALWAYS, StencilValue::TerrainStencilValue, 0xFF);
    terrain.draw(*terrain_shader, RenderPassType::MaterialRequired);
    gl_cache->set_stencil_write_mask(0x00);
    // ARC_POP_RENDER_TAG();

    // Reset state
    gl_cache->set_stencil_test(false);

    // Render pass output
    GeometryPassOutput pass_output;
    pass_output.output_gbuffer = gbuffer.get();
    return pass_output;
}
}