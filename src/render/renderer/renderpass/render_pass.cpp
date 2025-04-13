#include "render_pass.hpp"

#include <render/renderer/gl_cache.hpp>
#include <scene/scene.hpp>

namespace xen {
RenderPass::RenderPass(Scene& scene) : gl_cache(&GLCache::get()), active_scene(&scene) {}
}