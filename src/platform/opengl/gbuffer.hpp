#pragma once

#include "framebuffer.hpp"

namespace xen {
class GBuffer : public Framebuffer {
private:
    // 0 RGBA8  ->       albedo.r     albedo.g        albedo.b             albedo's alpha       (can replaced with
    // emission colour for emissive fragments) 1 RGB32F ->       normal.x     normal.y        normal.z 2 RGBA8  ->
    // metallic     roughness       ambientOcclusion     emissionIntensity
    std::array<Texture, 3> gbuffer_render_targets;

public:
    GBuffer(Vector2ui const& extent);
    ~GBuffer();

    [[nodiscard]] Texture* get_albedo() { return &gbuffer_render_targets[0]; }
    [[nodiscard]] Texture* get_normal() { return &gbuffer_render_targets[1]; }
    [[nodiscard]] Texture* get_material_info() { return &gbuffer_render_targets[2]; }

private:
    void init();
};
}