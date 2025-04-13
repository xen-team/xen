// #include "debug_draw_3d.hpp"

// #include <render/shader.hpp>
// #include <render/renderer/gl_cache.hpp>
// #include <platform/opengl/vertex_array.hpp>
// #include <platform/opengl/buffer.hpp>
// #include <utils/shader_loader.hpp>
// #include <scene/components/camera.hpp>

// namespace xen {
// GLCache* DebugDraw3D::gl_cache = nullptr;
// Shader* DebugDraw3D::line_shader = nullptr;
// std::unique_ptr<VertexArray> DebugDraw3D::line_vertex_array = nullptr;
// std::unique_ptr<Buffer> DebugDraw3D::line_vertex_buffer = nullptr;
// uint32_t DebugDraw3D::line_vertex_count = 0;
// std::unique_ptr<LineVertex> DebugDraw3D::line_vertex_buffer_base = nullptr;
// LineVertex* DebugDraw3D::line_vertex_buffer_ptr = nullptr;
// float DebugDraw3D::line_thickness = 3.0f;

// void DebugDraw3D::init()
// {
//     gl_cache = &GLCache::get();
//     line_shader = ShaderLoader::load_shader("debug_line.glsl");
//     line_vertex_array = std::make_unique<VertexArray>();
//     line_vertex_buffer = std::make_unique<Buffer>(
//         max_line_vertices * sizeof(LineVertex)
//     ); // Dynamic GPU Data that can we can update with our current s_LineVertexBuffer
//     // line_vertex_buffer->SetComponentCount(3);
//     line_vertex_array->add_vertex_layout(line_vertex_buffer, 0, 6 * sizeof(float), 0);
//     line_vertex_array->add_vertex_layout(line_vertex_buffer, 1, 6 * sizeof(float), 3 * sizeof(float));
//     line_vertex_buffer_base = std::make_unique<LineVertex>(max_line_vertices);
// }

// void DebugDraw3D::begin_batch()
// {
//     s_LineVertexCount = 0;
//     s_LineVertexBufferPtr = s_LineVertexBufferBase;
// }

// void DebugDraw3D::FlushBatch(ICamera* camera)
// {
//     if (s_LineVertexCount) {
//         uint32_t dataSize = (uint32_t)((uint8_t*)s_LineVertexBufferPtr - (uint8_t*)s_LineVertexBufferBase);
//         s_LineVertexBuffer->SetData(s_LineVertexBufferBase, dataSize);

//         s_GLCache->SetLineSmooth(true);
//         s_GLCache->SetLineWidth(s_LineThickness);
//         s_GLCache->SetShader(s_LineShader);
//         s_LineShader->SetUniform("viewProjection", camera->GetProjectionMatrix() * camera->GetViewMatrix());
//         s_LineVertexArray->Bind();
//         glDrawArrays(GL_LINES, 0, s_LineVertexCount);
//     }
// }

// void DebugDraw3D::QueueLine(glm::vec3 const& lineStart, glm::vec3 const& lineEnd, glm::vec3 const& colour)
// {
//     if (s_LineVertexCount >= s_MaxLineVertices) {
//         ARC_ASSERT(false, "DebugDraw3D hit line limit, won't render additional lines. Up the limit if you need it!");
//         return;
//     }

//     s_LineVertexBufferPtr->Position = lineStart;
//     s_LineVertexBufferPtr->Colour = colour;
//     s_LineVertexBufferPtr++;

//     s_LineVertexBufferPtr->Position = lineEnd;
//     s_LineVertexBufferPtr->Colour = colour;
//     s_LineVertexBufferPtr++;

//     s_LineVertexCount += 2;
// }

// void DebugDraw3D::QueueBox(glm::vec3 const& boxPos, glm::vec3 const& boxScale, glm::vec3 const& colour)
// {
//     glm::vec3 boxScaleHalf = boxScale * 0.5f;
//     QueueLine(
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z - boxScaleHalf.z),
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z - boxScaleHalf.z), colour
//     );
//     QueueLine(
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z - boxScaleHalf.z),
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z - boxScaleHalf.z), colour
//     );
//     QueueLine(
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z - boxScaleHalf.z),
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z - boxScaleHalf.z), colour
//     );
//     QueueLine(
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z - boxScaleHalf.z),
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z - boxScaleHalf.z), colour
//     );

//     QueueLine(
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z + boxScaleHalf.z),
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z + boxScaleHalf.z), colour
//     );
//     QueueLine(
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z + boxScaleHalf.z),
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z + boxScaleHalf.z), colour
//     );
//     QueueLine(
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z + boxScaleHalf.z),
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z + boxScaleHalf.z), colour
//     );
//     QueueLine(
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z + boxScaleHalf.z),
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z + boxScaleHalf.z), colour
//     );

//     QueueLine(
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z - boxScaleHalf.z),
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z + boxScaleHalf.z), colour
//     );
//     QueueLine(
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z - boxScaleHalf.z),
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y - boxScaleHalf.y, boxPos.z + boxScaleHalf.z), colour
//     );

//     QueueLine(
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z - boxScaleHalf.z),
//         glm::vec3(boxPos.x - boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z + boxScaleHalf.z), colour
//     );
//     QueueLine(
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z - boxScaleHalf.z),
//         glm::vec3(boxPos.x + boxScaleHalf.x, boxPos.y + boxScaleHalf.y, boxPos.z + boxScaleHalf.z), colour
//     );
// }

// void DebugDraw3D::QueueBox(glm::mat4 const& transform, glm::vec3 const& colour) {}

// void DebugDraw3D::SetLineThickness(float thickness)
// {
//     s_LineThickness = thickness;
// }
// }