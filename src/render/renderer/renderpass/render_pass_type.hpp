#pragma once

#include <platform/opengl/framebuffer.hpp>
#include <platform/opengl/gbuffer.hpp>

namespace xen {
class Cubemap;

enum class RenderPassType : uint8_t { MaterialRequired, NoMaterialRequired };

struct PostProcessPassOutput {
    Framebuffer* out_framebuffer;
};

struct ShadowmapPassOutput {
    Matrix4 directional_light_view_proj;
    Framebuffer* directional_shadowmap_framebuffer = nullptr;
    float directional_shadowmap_bias;

    Matrix4 spot_light_view_proj;
    Framebuffer* spot_light_shadowmap_framebuffer = nullptr;
    float spot_light_shadowmap_bias;

    bool has_point_light_shadows; // Need to have this since the point light shadow cubemap always needs to be bound
                                  // even if we never use it (thanks to the OpenGL Driver)
    Cubemap* point_light_shadow_cubemap = nullptr;
    float point_light_shadowmap_bias;
    float point_light_far_plane;
};

struct LightingPassOutput {
    Framebuffer* output_framebuffer = nullptr;
};

struct WaterPassOutput {
    Framebuffer* output_framebuffer = nullptr;
};

struct GeometryPassOutput {
    GBuffer* output_gbuffer = nullptr;
};

struct PreLightingPassOutput {
    Texture* ssao_texture = nullptr;
};

struct EditorPassOutput {
    Framebuffer* out_framebuffer = nullptr;
};
}