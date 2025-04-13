#include "renderer.hpp"

#include <algorithm>
#include <render/shader.hpp>
#include <render/mesh/model.hpp>
#include <render/mesh/mesh.hpp>
#include <render/renderer/gl_cache.hpp>
#include <render/mesh/common/cube.hpp>
#include <render/mesh/common/quad.hpp>
#include <animation/pose_animator.hpp>
#include <scene/components/camera.hpp>

namespace xen {
Renderer::Renderer() :
    ndc_plane{std::make_unique<Quad>()}, ndc_cube{std::make_unique<Cube>()}, gl_cache(&GLCache::get())
{
    float max_anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_anisotropy);
    renderer_data.max_anisotropy = max_anisotropy;

    // Debugdraw3D::Init();
}

void Renderer::begin_frame()
{
    current_draw_call_count = 0;
    current_meshes_drawn_count = 0;
    current_quads_drawn_count = 0;

    // Debugdraw3D::BeginBatch();
}

void Renderer::end_frame()
{
    renderer_data.draw_call_count = current_draw_call_count;
    renderer_data.meshes_drawn_count = current_meshes_drawn_count;
    renderer_data.quads_drawn_count = current_quads_drawn_count;
}

void Renderer::queue_quad(Vector3f const& position, Vector2f const& size, Texture& texture)
{
    Matrix4 const transform = Matrix4(1.f).translate(position) * Matrix4(1.f).scale(Vector3f{size.x, size.y, 1.0f});
    queue_quad(transform, texture);
}

void Renderer::queue_quad(
    Matrix4 const& transform, Texture& texture
) // TODO: Should use batch rendering to efficiently render quads together
{
    quad_draw_call_queue.emplace_back(QuadDrawCallInfo{&texture, transform});
}

void Renderer::queue_mesh(
    Model& model, Matrix4 const& transform, PoseAnimator* animator, bool is_transparent, bool cull_backface
)
{
    if (is_transparent) {
        if (animator) {
            transparent_skinned_mesh_draw_call_queue.emplace_back(
                MeshDrawCallInfo{&model, animator, transform, cull_backface}
            );
        }
        else {
            transparent_mesh_draw_call_queue.emplace_back(MeshDrawCallInfo{&model, animator, transform, cull_backface});
        }
    }
    else {
        if (animator) {
            opaque_skinned_mesh_draw_call_queue.emplace_back(
                MeshDrawCallInfo{&model, animator, transform, cull_backface}
            );
        }
        else {
            opaque_mesh_draw_call_queue.emplace_back(MeshDrawCallInfo{&model, nullptr, transform, cull_backface});
        }
    }
}

void Renderer::flush_opaque_skinned_meshes(CameraComponent& camera, RenderPassType pass, Shader& skinned_shader)
{
    if (!opaque_skinned_mesh_draw_call_queue.empty()) {
        gl_cache->set_shader(skinned_shader);
        bind_model_camera_info(camera, skinned_shader);
        setup_opaque_render_state();

        while (!opaque_skinned_mesh_draw_call_queue.empty()) {
            MeshDrawCallInfo& current = opaque_skinned_mesh_draw_call_queue.front();

            gl_cache->set_face_cull(current.cull_backface);
            setup_model_matrix(skinned_shader, current, pass);
            setup_bone_matrices(skinned_shader, current);
            current.model->draw(skinned_shader, pass);
            current_draw_call_count++;
            current_meshes_drawn_count++;

            opaque_skinned_mesh_draw_call_queue.pop_front();
        }
    }
}

void Renderer::flush_opaque_non_skinned_meshes(CameraComponent& camera, RenderPassType pass, Shader& shader)
{
    if (!opaque_mesh_draw_call_queue.empty()) {
        gl_cache->set_shader(shader);
        bind_model_camera_info(camera, shader);
        setup_opaque_render_state();

        while (!opaque_mesh_draw_call_queue.empty()) {
            MeshDrawCallInfo& current = opaque_mesh_draw_call_queue.front();

            gl_cache->set_face_cull(current.cull_backface);
            setup_model_matrix(shader, current, pass);
            current.model->draw(shader, pass);
            current_draw_call_count++;
            current_meshes_drawn_count++;

            opaque_mesh_draw_call_queue.pop_front();
        }
    }
}

void Renderer::flush_transparent_skinned_meshes(CameraComponent& camera, RenderPassType pass, Shader& skinned_shader)
{
    // Sort from back to front, does not account for rotations, scaling, or animation
    if (!transparent_skinned_mesh_draw_call_queue.empty()) {
        gl_cache->set_shader(skinned_shader);
        bind_model_camera_info(camera, skinned_shader);
        setup_opaque_render_state();

        std::ranges::sort(
            transparent_skinned_mesh_draw_call_queue,
            [camera](MeshDrawCallInfo& a, MeshDrawCallInfo& b) -> bool {
                auto const& camera_pos = camera.get_position();
                return (camera_pos - Vector3f(a.transform[3])).length_squared() >
                       (camera_pos - Vector3f(b.transform[3]))
                           .length_squared(); // transform[3] - Gets the translation part of the matrix
            }
        );
        while (!transparent_skinned_mesh_draw_call_queue.empty()) {
            MeshDrawCallInfo& current = transparent_skinned_mesh_draw_call_queue.front();

            gl_cache->set_face_cull(current.cull_backface);
            setup_model_matrix(skinned_shader, current, pass);
            setup_bone_matrices(skinned_shader, current);
            current.model->draw(skinned_shader, pass);
            current_draw_call_count++;
            current_meshes_drawn_count++;

            transparent_skinned_mesh_draw_call_queue.pop_front();
        }
    }
}

void Renderer::flush_transparent_non_skinned_meshes(CameraComponent& camera, RenderPassType pass, Shader& shader)
{
    // Sort from back to front, does not account for rotations or scaling
    if (!transparent_mesh_draw_call_queue.empty()) {

        gl_cache->set_shader(shader);
        bind_model_camera_info(camera, shader);
        setup_opaque_render_state();

        std::ranges::sort(transparent_mesh_draw_call_queue, [camera](MeshDrawCallInfo& a, MeshDrawCallInfo& b) -> bool {
            auto const& camera_pos = camera.get_position();
            return (camera_pos - Vector3f(a.transform[3])).length_squared() >
                   (camera_pos - Vector3f(b.transform[3]))
                       .length_squared(); // transform[3] - Gets the translation part of the matrix
        });
        while (!transparent_mesh_draw_call_queue.empty()) {
            MeshDrawCallInfo& current = transparent_mesh_draw_call_queue.front();

            gl_cache->set_face_cull(current.cull_backface);
            setup_model_matrix(shader, current, pass);
            current.model->draw(shader, pass);
            current_draw_call_count++;
            current_meshes_drawn_count++;

            transparent_mesh_draw_call_queue.pop_front();
        }
    }
}

void Renderer::flush_quads(CameraComponent& camera, Shader& shader)
{
    if (!quad_draw_call_queue.empty()) {
        gl_cache->set_shader(shader);
        bind_quad_camera_info(camera, shader);
        static Quad localQuad(false);
        setup_quad_render_state();

        while (!quad_draw_call_queue.empty()) {
            QuadDrawCallInfo& current = quad_draw_call_queue.front();

            current.texture->bind(5);
            shader.set_uniform("sprite", 5);
            setup_model_matrix(shader, current);
            localQuad.draw();
            current_draw_call_count++;
            current_quads_drawn_count++;

            quad_draw_call_queue.pop_front();
        }
    }
}

void Renderer::draw_ndc_plane()
{
    ndc_plane->draw();
    current_draw_call_count++;
}

void Renderer::draw_ndc_cube()
{
    ndc_cube->draw();
    current_draw_call_count++;
}

RendererData const& Renderer::get_renderer_data()
{
    return renderer_data;
}

void Renderer::bind_model_camera_info(CameraComponent& camera, Shader& shader)
{
    shader.set_uniform("viewPos", camera.get_position());
    shader.set_uniform("view", camera.get_view());
    shader.set_uniform("projection", camera.get_projection());
}

void Renderer::bind_quad_camera_info(CameraComponent& camera, Shader& shader)
{
    shader.set_uniform("view", camera.get_view());
    shader.set_uniform("projection", camera.get_projection());
}

void Renderer::setup_model_matrix(Shader& shader, MeshDrawCallInfo& draw_call_info, RenderPassType pass)
{
    shader.set_uniform("model", draw_call_info.transform);

    if (pass == RenderPassType::MaterialRequired) {
        Matrix3 const normal_matrix = draw_call_info.transform.inverse().transpose();
        shader.set_uniform("normalMatrix", normal_matrix);
    }
}

void Renderer::setup_model_matrix(Shader& shader, QuadDrawCallInfo& draw_call_info)
{
    shader.set_uniform("model", draw_call_info.transform);
}

void Renderer::setup_bone_matrices(Shader& shader, MeshDrawCallInfo& draw_call_info)
{
    if (draw_call_info.animator) {
        std::vector<Matrix4> const& matrices = draw_call_info.animator->GetFinalBoneMatrices();
        shader.set_uniform_array("bonesMatrices", matrices);
    }
}

void Renderer::setup_opaque_render_state()
{
    gl_cache->set_depth_test(true);
    gl_cache->set_blend(false);
    gl_cache->set_cull_face(GL_BACK);
}

void Renderer::setup_transparent_render_state()
{
    gl_cache->set_depth_test(true);
    gl_cache->set_blend(true);
    gl_cache->set_cull_face(GL_BACK);
    gl_cache->set_blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::setup_quad_render_state()
{
    gl_cache->set_depth_test(true);
    gl_cache->set_blend(false);
    gl_cache->set_face_cull(false);
}
}