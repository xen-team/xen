#pragma once

namespace xen {
class GLCache;
class Scene;

class RenderPass {
public:
    RenderPass(Scene& scene);
    virtual ~RenderPass() {};

protected:
    GLCache* gl_cache = nullptr;

    Scene* active_scene = nullptr;
};
}