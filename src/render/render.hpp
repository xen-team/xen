#pragma once

#include <system/windows.hpp>
#include <render/renderer/renderpass/deferred/deferred_geometry_pass.hpp>
#include <render/renderer/renderpass/deferred/deferred_light_pass.hpp>
#include <render/renderer/renderpass/forward/forward_probe_pass.hpp>
#include <render/renderer/renderpass/forward/forward_light_pass.hpp>
#include <render/renderer/renderpass/water_pass.hpp>
#include <render/renderer/renderpass/post_process_pass.hpp>
#include <render/renderer/renderpass/shadowmap_pass.hpp>
#include <render/renderer/renderpass/editor_pass.hpp>
#include <render/renderer/renderer.hpp>

namespace xen {
class Scene;
class Shader;

class XEN_API Render : public Module::Registrar<Render> {
    inline static bool const registered = Register("Render", Stage::Render, Depends<Windows>());

private:
    GLCache* gl_cache;

    Scene* linked_scene = nullptr;

    Texture* final_output_texture = nullptr;
    std::shared_ptr<Shader> passthrough_shader;

    std::unique_ptr<Renderer> renderer;

    std::unique_ptr<ShadowmapPass> shadowmap_pass;
    std::unique_ptr<PostProcessPass> post_process_pass;
    std::unique_ptr<WaterPass> water_pass;
    std::unique_ptr<EditorPass> editor_pass;

    std::unique_ptr<ForwardLightingPass> forward_lighting_pass;
    std::unique_ptr<ForwardProbePass> environment_probe_pass;

    std::unique_ptr<DeferredGeometryPass> deferred_geometry_pass;
    std::unique_ptr<DeferredLightingPass> deferred_lighting_pass;

    bool render_to_swapchain = true;

public:
    Render();

    void update() override;

    void init_renderer();

    void init_scene(Scene& scene);

    void set_render_to_swapchain(bool choice) { render_to_swapchain = choice; }

    [[nodiscard]] Renderer* get_renderer() { return renderer.get(); }
    [[nodiscard]] Texture* get_final_output_texture() { return final_output_texture; }
    [[nodiscard]] PostProcessPass* get_post_process_pass() { return post_process_pass.get(); }
    [[nodiscard]] EditorPass* get_editor_pass() { return editor_pass.get(); }
};
}