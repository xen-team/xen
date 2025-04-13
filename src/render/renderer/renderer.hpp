#pragma once

#include <render/renderer/renderpass/render_pass_type.hpp>
#include <engine/module.hpp>

namespace xen {
class GLCache;
class Model;
class Shader;
class CameraComponent;
class Cube;
class Quad;
class PoseAnimator;

struct RendererData {
    // Renderer Parameters
    float max_anisotropy;
    // Texture *BRDFLut;

    // Draw Call Statistics
    uint32_t draw_call_count;
    uint32_t meshes_drawn_count;
    uint32_t quads_drawn_count;
};

// TODO: Should eventually have a render ID and we can order drawcalls to avoid changing GPU state (shaders etc)
// Some sort of draw call bucketing system
struct MeshDrawCallInfo {
    Model* model = nullptr;
    PoseAnimator* animator = nullptr;
    Matrix4 transform;
    bool cull_backface;
};
struct QuadDrawCallInfo {
    Texture* texture;
    Matrix4 transform;
};

class Renderer  {
private:
    std::unique_ptr<Quad> ndc_plane;
    std::unique_ptr<Cube> ndc_cube;

    RendererData renderer_data;
    GLCache* gl_cache;

    std::deque<MeshDrawCallInfo> opaque_mesh_draw_call_queue;
    std::deque<MeshDrawCallInfo> opaque_skinned_mesh_draw_call_queue;
    std::deque<MeshDrawCallInfo> transparent_mesh_draw_call_queue;
    std::deque<MeshDrawCallInfo> transparent_skinned_mesh_draw_call_queue;
    std::deque<QuadDrawCallInfo> quad_draw_call_queue;

    uint32_t current_draw_call_count = 0;
    uint32_t current_meshes_drawn_count = 0;
    uint32_t current_quads_drawn_count = 0;

public:
    Renderer();

    void begin_frame();
    void end_frame();

    void queue_mesh(
        Model& model, Matrix4 const& transform, PoseAnimator* animator = nullptr, bool is_transparent = false,
        bool cull_backface = true
    );
    void queue_quad(
        Vector3f const& position, Vector2f const& size, Texture& texture
    ); // TODO: Should use batch rendering to efficiently render quads together
    void queue_quad(
        Matrix4 const& transform, Texture& texture
    ); // TODO: Should use batch rendering to efficiently render quads together

    void flush_opaque_skinned_meshes(CameraComponent& camera, RenderPassType pass, Shader& skinned_shader);
    void flush_opaque_non_skinned_meshes(CameraComponent& camera, RenderPassType pass, Shader& shader);
    void flush_transparent_skinned_meshes(CameraComponent& camera, RenderPassType pass, Shader& skinned_shader);
    void flush_transparent_non_skinned_meshes(CameraComponent& camera, RenderPassType pass, Shader& shader);
    void flush_quads(CameraComponent& camera, Shader& shader);

    void draw_ndc_plane();
    void draw_ndc_cube();

    RendererData const& get_renderer_data();

private:
    void bind_model_camera_info(CameraComponent& camera, Shader& shader);
    void bind_quad_camera_info(CameraComponent& camera, Shader& shader);
    void setup_model_matrix(Shader& shader, MeshDrawCallInfo& draw_call_info, RenderPassType pass);
    void setup_model_matrix(Shader& shader, QuadDrawCallInfo& draw_call_info);
    void setup_bone_matrices(Shader& shader, MeshDrawCallInfo& draw_call_info);
    void setup_opaque_render_state();
    void setup_transparent_render_state();
    void setup_quad_render_state();
};
}