#pragma once

#include <render/renderer/renderpass/render_pass.hpp>
#include <render/renderer/renderpass/render_pass_type.hpp>

namespace xen {
class Shader;
class Scene;
class CameraComponent;
class GBuffer;

class DeferredGeometryPass : public RenderPass {
private:
    std::shared_ptr<GBuffer> gbuffer;

    std::shared_ptr<Shader> model_shader;
    std::shared_ptr<Shader> skinned_model_shader;
    std::shared_ptr<Shader> terrain_shader;

public:
    DeferredGeometryPass(Scene& scene);
    DeferredGeometryPass(Scene& scene, GBuffer& custom_gbuffer);

    GeometryPassOutput execute_geometry_pass(CameraComponent& camera, bool render_only_static);
};
}