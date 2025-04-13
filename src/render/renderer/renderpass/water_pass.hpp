#pragma once

#include <render/renderer/renderpass/render_pass.hpp>
#include <render/renderer/renderpass/render_pass_type.hpp>
#include <render/mesh/common/quad.hpp>
#include <utils/timers.hpp>

namespace xen {
class Shader;
class Scene;
class CameraComponent;

class WaterPass : public RenderPass {
private:
    bool water_enabled = true;

    std::shared_ptr<Shader> water_shader;
    Quad water_plane;
    SimpleTimer effects_timer;

public:
    WaterPass(Scene& scene);

    WaterPassOutput execute(ShadowmapPassOutput& shadowmap_data, Framebuffer& framebuffer, CameraComponent& camera);
};
}