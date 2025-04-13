#pragma once

#include "math/math.hpp"

namespace xen {
enum class BlendFactor : uint8_t {
    NONE = 0,
    ZERO,
    ONE,
    ONE_MINUS_SRC_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DST_ALPHA,
    ONE_MINUS_DST_ALPHA,
    DST_COLOR,
    ONE_MINUS_DST_COLOR,
    CONSTANT_COLOR,
    ONE_MINUS_CONSTANT_COLOR,
    CONSTANT_ALPHA,
    ONE_MINUS_CONSTANT_ALPHA,
};

enum class DepthFunction : uint8_t {
    EQUAL = 0,
    NOT_EQUAL,
    LESS,
    GREATER,
    LESS_EQUAL,
    GREATER_EQUAL,
    ALWAYS,
    NEVER,
};

enum class RenderPrimitive : uint8_t {
    POINTS = 0,
    LINE_STRIP,
    LINE_LOOP,
    LINES,
    LINE_STRIP_ADJACENCY,
    LINES_ADJACENCY,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
    TRIANGLES,
    TRIANGLE_STRIP_ADJACENCY,
    TRIANGLES_ADJACENCY,
    PATCHES,
};

class Renderer {
    bool depth_buffer = false;
    uint16_t clear_mask = 0;

public:
    constexpr Renderer();

    void draw_vertices(RenderPrimitive primitive, size_t count, size_t offset);

    void draw_indices(RenderPrimitive primitive, size_t count, size_t offset);

    void draw_vertices_instanced(
        RenderPrimitive primitive, size_t count, size_t offset, size_t instance_count, size_t instance_base
    );

    void draw_indices_instanced(
        RenderPrimitive primitive, size_t count, size_t offset, size_t instance_count, size_t instance_base
    );

    void draw_indices_base_vertex(RenderPrimitive primitive, size_t count, size_t offset, size_t base);

    void draw_indices_base_vertex_instanced(
        RenderPrimitive primitive, size_t count, size_t offset, size_t base, size_t instance_count, size_t instance_base
    );

    static void default_vertex_attribute(size_t index, float v);
    static void default_vertex_attribute(size_t index, Vector2f const& vec);
    static void default_vertex_attribute(size_t index, Vector3f const& vec);
    static void default_vertex_attribute(size_t index, Vector4f const& vec);
    static void default_vertex_attribute(size_t index, Matrix2 const& mat);
    static void default_vertex_attribute(size_t index, Matrix3 const& mat);
    static void default_vertex_attribute(size_t index, Matrix4 const& mat);

    void clear() const;

    void flush() const;

    void finish() const;

    void set_viewport(int x, int y, int width, int height) const;

    Renderer& use_clip_distance(size_t count);
    Renderer& use_seamless_cube_maps(bool value = true);
    Renderer& use_color_mask(bool r, bool g, bool b, bool a);
    Renderer& use_depth_buffer_mask(bool value = true);
    Renderer& use_sampling(bool value = true);
    Renderer& use_depth_buffer(bool value = true);
    Renderer& use_depth_clamp(bool value = true);
    Renderer& use_reversed_depth(bool value = true);
    Renderer& use_depth_function(DepthFunction function);
    Renderer& use_culling(bool value = true, bool counter_clock_wise = true, bool cull_back = true);
    Renderer& use_clear_color(float r, float g, float b, float a = 0.0f);
    Renderer& use_blend_factors(BlendFactor src, BlendFactor dist);
    Renderer& use_anisotropic_filtering(float factor);
    [[nodiscard]] constexpr float get_largest_anisotropic_factor() const;
};
}