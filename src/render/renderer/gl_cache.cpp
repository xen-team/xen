#include "gl_cache.hpp"

#include <render/shader.hpp>

namespace xen {
void GLCache::init()
{
    set_depth_test(true);
    set_face_cull(true);
    set_line_smooth(true);
}

void GLCache::set_depth_test(bool choice)
{
    if (depth_test != choice) {
        depth_test = choice;
        if (depth_test) {
            GL_CALL(glEnable(GL_DEPTH_TEST));
        }
        else {
            GL_CALL(glDisable(GL_DEPTH_TEST));
        }
    }
}

void GLCache::set_stencil_test(bool choice)
{
    if (stencil_test != choice) {
        stencil_test = choice;
        if (stencil_test) {
            GL_CALL(glEnable(GL_STENCIL_TEST));
        }
        else {
            GL_CALL(glDisable(GL_STENCIL_TEST));
        }
    }
}

void GLCache::set_blend(bool choice)
{
    if (blend != choice) {
        blend = choice;
        if (blend) {
            GL_CALL(glEnable(GL_BLEND));
        }
        else {
            GL_CALL(glDisable(GL_BLEND));
        }
    }
}

void GLCache::set_face_cull(bool choice)
{
    if (cull != choice) {
        cull = choice;
        if (cull) {
            GL_CALL(glEnable(GL_CULL_FACE));
        }
        else {
            GL_CALL(glDisable(GL_CULL_FACE));
        }
    }
}

void GLCache::set_multisample(bool choice)
{
    if (multisample != choice) {
        multisample = choice;
        if (multisample) {
            GL_CALL(glEnable(GL_MULTISAMPLE));
        }
        else {
            GL_CALL(glDisable(GL_MULTISAMPLE));
        }
    }
}

void GLCache::set_uses_clip_plane(bool choice)
{
    if (uses_clip_plane != choice) {
        uses_clip_plane = choice;
        if (uses_clip_plane) {
            GL_CALL(glEnable(GL_CLIP_DISTANCE0));
        }
        else {
            GL_CALL(glDisable(GL_CLIP_DISTANCE0));
        }
    }
}

void GLCache::set_line_smooth(bool choice)
{
    if (line_smooth != choice) {
        line_smooth = choice;
        if (line_smooth) {
            GL_CALL(glEnable(GL_LINE_SMOOTH));
        }
        else {
            GL_CALL(glDisable(GL_LINE_SMOOTH));
        }
    }
}

void GLCache::set_depth_func(GLenum depth_func)
{
    if (this->depth_func != depth_func) {
        this->depth_func = depth_func;
        GL_CALL(glDepthFunc(this->depth_func));
    }
}

void GLCache::set_stencil_func(GLenum func, int frag_value, uint32_t stencil_bitmask)
{
    if (stencil_test_func != func || stencil_frag_value != frag_value || stencil_func_bitmask != stencil_bitmask) {
        stencil_test_func = func;
        stencil_frag_value = frag_value;
        stencil_func_bitmask = stencil_bitmask;

        GL_CALL(glStencilFuncSeparate(GL_FRONT_AND_BACK, stencil_test_func, stencil_frag_value, stencil_func_bitmask));
    }
}

void GLCache::set_stencil_op(GLenum stencil_fail_operation, GLenum depth_fail_operation, GLenum depth_pass_operation)
{
    if (this->stencil_fail_operation != stencil_fail_operation || this->depth_fail_operation != depth_pass_operation ||
        this->depth_pass_operation != depth_pass_operation) {
        this->stencil_fail_operation = stencil_fail_operation;
        this->depth_fail_operation = depth_fail_operation;
        this->depth_pass_operation = depth_pass_operation;

        GL_CALL(glStencilOpSeparate(
            GL_FRONT_AND_BACK, this->stencil_fail_operation, this->depth_fail_operation, this->depth_pass_operation
        ));
    }
}

void GLCache::set_stencil_write_mask(uint32_t bitmask)
{
    if (stencil_write_bitmask != bitmask) {
        stencil_write_bitmask = bitmask;
        GL_CALL(glStencilMaskSeparate(GL_FRONT_AND_BACK, stencil_write_bitmask));
    }
}

void GLCache::set_color_mask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    if (red_mask != red || green_mask != green || blue_mask != blue || alpha_mask != alpha) {
        red_mask = red;
        green_mask = green;
        blue_mask = blue;
        alpha_mask = alpha;
        GL_CALL(glColorMask(red_mask, green_mask, blue_mask, alpha_mask));
    }
}

void GLCache::set_blend_func(GLenum src, GLenum dst)
{
    if (blend_src != src || blend_dst != dst) {
        blend_src = src;
        blend_dst = dst;
        GL_CALL(glBlendFunc(blend_src, blend_dst));
    }
}

void GLCache::set_cull_face(GLenum face_to_cull)
{
    if (multisample != face_to_cull) {
        multisample = face_to_cull;
        GL_CALL(glCullFace(multisample));
    }
}

void GLCache::set_clip_plane(Vector4f const& clip_plane)
{
    active_clip_plane = clip_plane;
}

void GLCache::set_line_width(float thickness)
{
    if (line_thickness != thickness) {
        line_thickness = thickness;
        GL_CALL(glLineWidth(line_thickness));
    }
}

void GLCache::set_shader(Shader& shader)
{
    if (active_shader_id != shader.get_shader_id()) {
        active_shader_id = shader.get_shader_id();
        shader.enable();
    }
}

void GLCache::set_shader(uint32_t shader_id)
{
    if (active_shader_id != shader_id) {
        active_shader_id = shader_id;
        GL_CALL(glUseProgram(shader_id));
    }
}
}