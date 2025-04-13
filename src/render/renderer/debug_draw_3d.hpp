// #pragma once
// /*
//         This class can be used to render lines and shapes. This should not be used for gameplay purposes, because it
//    isn't performant and the lines aren't anti aliased properly They are just lines for debugging purposes, displaying
//    debug information in the editor, or scripting purposes when testing raycasts and other things
// */

// // Need totally overwrite

// namespace xen {
// class CameraComponent;
// class GLCache;
// class Shader;
// class VertexArray;
// class Buffer;

// struct LineVertex {
//     Vector3f position;
//     Vector3f color;
// };

// class DebugDraw3D {
// private:
//     static GLCache* gl_cache;

//     static Shader* line_shader;
//     static std::unique_ptr<VertexArray> line_vertex_array;
//     static std::unique_ptr<Buffer> line_vertex_buffer;
//     static std::unique_ptr<LineVertex> line_vertex_buffer_base;
//     static LineVertex* line_vertex_buffer_ptr;
//     static uint32_t line_vertex_count;

//     static uint32_t const max_line_vertices =
//         2000; // 1000 lines limit (can be jacked up or re-visited to resize properly)
//     static float line_thickness;

// public:
//     static void init();

//     static void begin_batch();
//     static void flush_batch(CameraComponent& camera);

//     static void line(Vector3f const& line_start, Vector3f const& line_end, Vector3f const& color);
//     static void box(Vector3f const& box_pos, Vector3f const& box_scale, Vector3f const& color);
//     static void box(Matrix4 const& transform, Vector3f const& color);

//     static void set_line_thickness(float thickness);
// };
// }