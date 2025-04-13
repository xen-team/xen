#include "render.hpp"

#include <system/windows.hpp>
#include <render/shader.hpp>
#include <render/renderer/gl_cache.hpp>
#include <render/renderer/renderer.hpp>
#include <render/mesh/common/cube.hpp>
#include <utils/shader_loader.hpp>
#include <utils/texture_loader.hpp>
#include <scene/scene.hpp>

// #ifdef ARC_DEV_BUILD
// #include <Arcane/Platform/OpenGL/GPUTimerManager.h>
// #endif

namespace xen {
void Render::init_renderer()
{
    renderer = std::make_unique<Renderer>();
}

void Render::init_scene(Scene& scene)
{
    static bool first_pass = true;
    if (first_pass) {
        GL_CALL(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));

        TextureLoader::initialize_default_textures();

        first_pass = false;
    }

    gl_cache->init();

    shadowmap_pass = std::make_unique<ShadowmapPass>(scene);
    post_process_pass = std::make_unique<PostProcessPass>(scene);
    water_pass = std::make_unique<WaterPass>(scene);
    editor_pass = std::make_unique<EditorPass>(scene);
    forward_lighting_pass = std::make_unique<ForwardLightingPass>(scene, false);
    environment_probe_pass = std::make_unique<ForwardProbePass>(scene);

#if FORWARD_RENDER
    forward_lighting_pass = std::make_unique<ForwardLightingPass>(scene, true);
#else
    deferred_geometry_pass = std::make_unique<DeferredGeometryPass>(scene);
#endif
    deferred_lighting_pass = std::make_unique<DeferredLightingPass>(scene);

    environment_probe_pass->pregenerate_ibl();
    environment_probe_pass->pregenerate_probes();

    linked_scene = &scene;

    passthrough_shader = ShaderLoader::load_shader("post_process/copy.glsl");
}

Render::Render() : gl_cache(&GLCache::get())
{

#ifdef ARC_DEV_BUILD
#if FORWARD_RENDER
    m_ShadowPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Shadow Map Generation Pass (GPU)"));
    m_ForwardOpaquePassTimer = GPUTimerManager::CreateGPUTimer(std::string("Forward Opaque Pass (GPU)"));
    m_WaterPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Water Pass (GPU)"));
    m_ForwardTransparentPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Forward Transparent Pass (GPU)"));
    m_PostProcessPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Post Process Pass (GPU)"));
    m_EditorPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Editor Pass (GPU)"));
#else
    m_ShadowPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Shadow Map Generation Pass (GPU)"));
    m_DeferredGeometryPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Deferred Geometry Pass (GPU)"));
    m_SSAOPassTimer = GPUTimerManager::CreateGPUTimer(std::string("SSAO Pass (GPU)"));
    m_DeferredLightingPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Deferred Lighting Pass (GPU)"));
    m_WaterPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Water Pass (GPU)"));
    m_PostGBufferForwardPassTimer =
        GPUTimerManager::CreateGPUTimer(std::string("Post GBuffer Forward Transparent Pass (GPU)"));
    m_PostProcessPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Post Process Pass (GPU)"));
    m_EditorPassTimer = GPUTimerManager::CreateGPUTimer(std::string("Editor Pass (GPU)"));
#endif
#endif
}

void Render::update()
{
    renderer->begin_frame();

#if FORWARD_RENDER
    /* Forward Rendering */
    ARC_PUSH_RENDER_TAG("Shadow Pass");
    ARC_GPU_TIMER_BEGIN(m_ShadowPassTimer);
    ShadowmapPassOutput shadowmapOutput = m_ShadowmapPass.GenerateShadowmaps(m_ActiveScene->GetCamera(), false);
    ARC_GPU_TIMER_END(m_ShadowPassTimer);
    ARC_POP_RENDER_TAG();

    ARC_PUSH_RENDER_TAG("Forward Opaque Pass");
    ARC_GPU_TIMER_BEGIN(m_ForwardOpaquePassTimer);
    LightingPassOutput lightingOutput =
        m_ForwardLightingPass.ExecuteOpaqueLightingPass(shadowmapOutput, m_ActiveScene->GetCamera(), false, true);
    ARC_GPU_TIMER_END(m_ForwardOpaquePassTimer);
    ARC_POP_RENDER_TAG();

    ARC_PUSH_RENDER_TAG("Water Pass");
    ARC_GPU_TIMER_BEGIN(m_WaterPassTimer);
    WaterPassOutput waterOutput =
        m_WaterPass.ExecuteWaterPass(shadowmapOutput, lightingOutput.outputFramebuffer, m_ActiveScene->GetCamera());
    ARC_GPU_TIMER_END(m_WaterPassTimer);
    ARC_POP_RENDER_TAG();

    ARC_PUSH_RENDER_TAG("Forward Transparent Pass");
    ARC_GPU_TIMER_BEGIN(m_ForwardTransparentPassTimer);
    LightingPassOutput postTransparencyOutput = m_ForwardLightingPass.ExecuteTransparentLightingPass(
        shadowmapOutput, waterOutput.outputFramebuffer, m_ActiveScene->GetCamera(), false, true
    );
    ARC_GPU_TIMER_END(m_ForwardTransparentPassTimer);
    ARC_POP_RENDER_TAG();

    ARC_PUSH_RENDER_TAG("Post Process Pass");
    ARC_GPU_TIMER_BEGIN(m_PostProcessPassTimer);
    PostProcessPassOutput postProcessOutput =
        m_PostProcessPass.ExecutePostProcessPass(postTransparencyOutput.outputFramebuffer);
    ARC_GPU_TIMER_END(m_PostProcessPassTimer);
    ARC_POP_RENDER_TAG();

    ARC_PUSH_RENDER_TAG("Editor Pass");
    ARC_GPU_TIMER_BEGIN(m_EditorPassTimer);
    Framebuffer* extraFramebuffer = postProcessOutput.outFramebuffer == m_PostProcessPass.GetFullRenderTarget() ?
                                        m_PostProcessPass.GetTonemappedNonLinearTarget() :
                                        m_PostProcessPass.GetFullRenderTarget();
    EditorPassOutput editorOutput = m_EditorPass.ExecuteEditorPass(
        postProcessOutput.outFramebuffer, m_PostProcessPass.GetResolveRenderTarget(), extraFramebuffer,
        m_ActiveScene->GetCamera()
    );
    ARC_GPU_TIMER_END(m_EditorPassTimer);
    ARC_POP_RENDER_TAG();
#else

    /* Deferred Rendering */
    // ARC_PUSH_RENDER_TAG("Shadow Pass");
    // ARC_GPU_TIMER_BEGIN(m_ShadowPassTimer);
    ShadowmapPassOutput shadowmap_out = shadowmap_pass->generate_shadowmaps(linked_scene->get_camera(), false);
    // ARC_GPU_TIMER_END(m_ShadowPassTimer);
    // ARC_POP_RENDER_TAG();

    // ARC_PUSH_RENDER_TAG("Deferred Geometry Pass (Opaque)");
    // ARC_GPU_TIMER_BEGIN(m_DeferredGeometryPassTimer);
    GeometryPassOutput geometry_out = deferred_geometry_pass->execute_geometry_pass(linked_scene->get_camera(), false);
    // ARC_GPU_TIMER_END(m_DeferredGeometryPassTimer);
    // ARC_POP_RENDER_TAG();

    // ARC_PUSH_RENDER_TAG("Pre-Lighting Pass");
    // ARC_GPU_TIMER_BEGIN(m_SSAOPassTimer);
    PreLightingPassOutput pre_lightning_out =
        post_process_pass->execute_pre_lighting_pass(*geometry_out.output_gbuffer, linked_scene->get_camera());
    // ARC_GPU_TIMER_END(m_SSAOPassTimer);
    // ARC_POP_RENDER_TAG();

    // ARC_PUSH_RENDER_TAG("Deferred Lighting Pass");
    // ARC_GPU_TIMER_BEGIN(m_DeferredLightingPassTimer);
    LightingPassOutput deferred_lighting_out = deferred_lighting_pass->execute_lighting_pass(
        shadowmap_out, *geometry_out.output_gbuffer, pre_lightning_out, linked_scene->get_camera(), true
    );
    // ARC_GPU_TIMER_END(m_DeferredLightingPassTimer);
    // ARC_POP_RENDER_TAG();

    // ARC_PUSH_RENDER_TAG("Water Pass");
    // ARC_GPU_TIMER_BEGIN(m_WaterPassTimer);
    WaterPassOutput water_out =
        water_pass->execute(shadowmap_out, *deferred_lighting_out.output_framebuffer, linked_scene->get_camera());
    // ARC_GPU_TIMER_END(m_WaterPassTimer);
    // ARC_POP_RENDER_TAG();

    // ARC_PUSH_RENDER_TAG("Post GBuffer Forward Pass (Transparent)");
    // ARC_GPU_TIMER_BEGIN(m_PostGBufferForwardPassTimer);
    LightingPassOutput post_gbuffer_forward = forward_lighting_pass->execute_transparent_lightning_pass(
        shadowmap_out, *water_out.output_framebuffer, linked_scene->get_camera(), false, true
    );
    // ARC_GPU_TIMER_END(m_PostGBufferForwardPassTimer);
    // ARC_POP_RENDER_TAG();

    // ARC_PUSH_RENDER_TAG("Post Process Pass");
    // ARC_GPU_TIMER_BEGIN(m_PostProcessPassTimer);
    PostProcessPassOutput post_process_out =
        post_process_pass->execute_post_process_pass(*post_gbuffer_forward.output_framebuffer);
    // ARC_GPU_TIMER_END(m_PostProcessPassTimer);
    // ARC_POP_RENDER_TAG();

    // ARC_PUSH_RENDER_TAG("Editor Pass");
    // ARC_GPU_TIMER_BEGIN(m_EditorPassTimer);
    Framebuffer& extra_framebuffer = post_process_out.out_framebuffer->get_framebuffer() ==
                                             post_process_pass->get_full_render_target().get_framebuffer() ?
                                         post_process_pass->get_tonemapped_non_linear_target() :
                                         post_process_pass->get_full_render_target();

    EditorPassOutput editor_out = editor_pass->execute(
        *post_process_out.out_framebuffer, post_process_pass->get_resolve_render_target(), extra_framebuffer,
        linked_scene->get_camera()
    );

    // ARC_GPU_TIMER_END(m_EditorPassTimer);
    // ARC_POP_RENDER_TAG();
#endif

    // Finally render the scene to the window's swapchain
    final_output_texture = &editor_out.out_framebuffer->get_color_texture();
    if (render_to_swapchain) {
        Window::bind();
        Window::clear_all();

        gl_cache->set_shader(*passthrough_shader);
        passthrough_shader->set_uniform("input_texture", 0);
        final_output_texture->bind(0);
        renderer->draw_ndc_plane();
    }

    renderer->end_frame();
}
}