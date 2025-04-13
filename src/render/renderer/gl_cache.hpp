#pragma once

#include <utils/classes.hpp>

namespace xen {
class Shader;

class GLCache : public NonCopyableMovable {
private:
    bool depth_test = false;
    bool stencil_test = false;
    bool blend = false;
    bool cull = false;
    bool multisample = false;
    bool uses_clip_plane = false;
    bool line_smooth = false;

    GLenum depth_func;

    GLenum stencil_test_func;
    int stencil_frag_value;
    uint32_t stencil_func_bitmask;

    GLenum stencil_fail_operation, depth_fail_operation, depth_pass_operation;
    uint32_t stencil_write_bitmask;

    GLboolean red_mask = GL_TRUE, blue_mask = GL_TRUE, green_mask = GL_TRUE, alpha_mask = GL_TRUE;

    GLenum blend_src, blend_dst;

    GLenum face_to_cull;

    Vector4f active_clip_plane = Vector4f(0.f);

    float line_thickness = -1.0f;

    uint32_t active_shader_id = 0;

public:
    void init();

    static GLCache& get()
    {
        static GLCache cache;
        return cache;
    }

    void set_depth_test(bool choice);
    void set_stencil_test(bool choice);
    void set_blend(bool choice);
    void set_face_cull(bool choice);
    void set_multisample(bool choice);
    void set_uses_clip_plane(bool choice);
    void set_line_smooth(bool choice);

    void set_depth_func(GLenum func);
    void set_stencil_func(GLenum func, int frag_value, uint32_t stencil_bitmask = 0xFF);
    void set_stencil_op(GLenum stencil_fail_operation, GLenum depth_fail_operation, GLenum depth_pass_operation);
    void set_stencil_write_mask(uint32_t bitmask);
    void set_color_mask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    void set_blend_func(GLenum src, GLenum dst);
    void set_cull_face(GLenum face_to_cull);
    void set_clip_plane(Vector4f const& clip_plane);
    void set_line_width(float thickness);

    void set_shader(Shader& shader);
    void set_shader(uint32_t shader_id);

    [[nodiscard]] bool get_uses_clip_plane() const { return uses_clip_plane; }
    [[nodiscard]] Vector4f get_active_clip_plane() const { return active_clip_plane; }
};
}