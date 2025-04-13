#include "mesh.hpp"

#include <platform/opengl/index_buffer.hpp>
#include <platform/opengl/vertex_array.hpp>

namespace xen {
void Mesh::draw() const
{
    GL_CALL(glBindVertexArray(vao));
    if (indices.size() > 0) {
        GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
        GL_CALL(glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr));
        GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }
    else {
        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(indices.size())));
    }
    GL_CALL(glBindVertexArray(0));
}

void Mesh::load_data(bool const interleaved)
{
    // Check for possible mesh initialization errors
#ifdef XEN_DEBUG
    {
        auto const vertex_count = positions.size();

        if (vertex_count == 0) {
            Log::warning("Render::Mesh"
                         " mesh doesn't contain any vertices");
        }
        if (uvs.size() != 0 && uvs.size() != vertex_count) {
            Log::warning("Render::Mesh"
                         " mesh UV count doesn't match the vertex count");
        }
        if (normals.size() != 0 && normals.size() != vertex_count) {
            Log::warning("Render::Mesh"
                         " mesh Normal count doesn't match the vertex count");
        }
        if (tangents.size() != 0 && tangents.size() != vertex_count) {
            Log::warning("Render::Mesh"
                         " mesh Tangent count doesn't match the vertex count");
        }
        if (bitangents.size() != 0 && bitangents.size() != vertex_count) {
            Log::warning("Render::Mesh"
                         " mesh Bitangent count doesn't match the vertex count");
        }
        if (bone_data.size() != 0 && bone_data.size() != vertex_count) {
            Log::warning("Render::Mesh"
                         " mesh Bone Data count doesn't match the vertex count");
        }
    }
#endif

    is_interleaved = interleaved;

    // Compute the component count
    buffer_component_count = 0;
    if (!positions.empty()) {
        buffer_component_count += 3;
    }
    if (!normals.empty()) {
        buffer_component_count += 3;
    }
    if (!uvs.empty()) {
        buffer_component_count += 2;
    }
    if (!tangents.empty()) {
        buffer_component_count += 3;
    }
    if (!bitangents.empty()) {
        buffer_component_count += 3;
    }
    if (!bone_data.empty()) {
        buffer_component_count += (2 * max_bones_per_vertex);
    }

    // Pre-process the mesh data in the format that was specified
    buffer_data.reserve(
        (3 * positions.size()) + (3 * normals.size()) + (2 * uvs.size()) + (3 * tangents.size()) +
        (3 * bitangents.size()) + (bone_data.size() * 2 * max_bones_per_vertex)
    );

    if (interleaved) {
        setup_interleaved_attributes();
    }
    else {
        for (auto const& position : positions) {
            buffer_data.emplace_back(position.x);
            buffer_data.emplace_back(position.y);
            buffer_data.emplace_back(position.z);
        }
        for (auto const& normal : normals) {
            buffer_data.emplace_back(normal.x);
            buffer_data.emplace_back(normal.y);
            buffer_data.emplace_back(normal.z);
        }
        for (auto const& uv : uvs) {
            buffer_data.emplace_back(uv.x);
            buffer_data.emplace_back(uv.y);
        }
        for (auto const& tangent : tangents) {
            buffer_data.emplace_back(tangent.x);
            buffer_data.emplace_back(tangent.y);
            buffer_data.emplace_back(tangent.z);
        }
        for (auto const& bitangent : bitangents) {
            buffer_data.emplace_back(bitangent.x);
            buffer_data.emplace_back(bitangent.y);
            buffer_data.emplace_back(bitangent.z);
        }
        for (auto const& i : bone_data) {
            for (int j = 0; j < max_bones_per_vertex; j++) {
                buffer_data.emplace_back(static_cast<int>(i.bone_ids[j]));
            }
        }
        for (auto const& i : bone_data) {
            for (int j = 0; j < max_bones_per_vertex; j++) {
                buffer_data.emplace_back(i.weights[j]);
            }
        }
    }
}

void Mesh::generate_gpu_data()
{
    GL_CALL(glGenVertexArrays(1, &vao));
    GL_CALL(glGenBuffers(1, &vbo));
    GL_CALL(glGenBuffers(1, &ibo));

    // Load data into the index buffer and vertex buffer
    GL_CALL(glBindVertexArray(vao));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferData(
        GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(buffer_data.size() * sizeof(float)), buffer_data.data(), GL_STATIC_DRAW
    ));
    if (!indices.empty()) {
        GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
        GL_CALL(glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(),
            GL_STATIC_DRAW
        ));
    }

    // Setup the format for the VAO
    if (is_interleaved) {
        setup_interleaved_attributes();
    }
    else {
        setup_separate_attributes();
    }

    GL_CALL(glBindVertexArray(0));
}

void Mesh::setup_interleaved_attributes()
{
    size_t const stride = buffer_component_count * sizeof(float);
    size_t offset = 0;

    std::array<std::tuple<int, int, GLenum, bool>, 9> const attributes = {{
        {0, 3, GL_FLOAT, true},                // positions
        {1, 3, GL_FLOAT, !normals.empty()},    // normals
        {2, 2, GL_FLOAT, !uvs.empty()},        // uvs
        {3, 3, GL_FLOAT, !tangents.empty()},   // tangents
        {4, 3, GL_FLOAT, !bitangents.empty()}, // bitangents
        {5, 4, GL_INT, !bone_data.empty()},    // bone indices
        {6, 4, GL_FLOAT, !bone_data.empty()}   // bone weights
    }};

    for (auto const& [index, size, type, enabled] : attributes) {
        if (enabled) {
            bool const is_int = (type == GL_INT);

            GL_CALL(glEnableVertexAttribArray(index));

            if (is_int) {
                GL_CALL(glVertexAttribIPointer(index, size, type, stride, (void*)offset));
            }
            else {
                GL_CALL(glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)offset));
            }
            offset += size * (is_int ? sizeof(int) : sizeof(float));
        }
    }
}

void Mesh::setup_separate_attributes()
{
    size_t offset = 0;

    std::array<std::tuple<int, int, GLenum, bool>, 7> const attributes = {{
        {0, 3, GL_FLOAT, true},                // positions
        {1, 3, GL_FLOAT, !normals.empty()},    // normals
        {2, 2, GL_FLOAT, !uvs.empty()},        // uvs
        {3, 3, GL_FLOAT, !tangents.empty()},   // tangents
        {4, 3, GL_FLOAT, !bitangents.empty()}, // bitangents
        {5, 4, GL_INT, !bone_data.empty()},    // bone indices
        {6, 4, GL_FLOAT, !bone_data.empty()}   // bone weights
    }};

    for (auto const& [index, size, type, enabled] : attributes) {
        if (enabled) {
            bool const is_int = (type == GL_INT);

            GL_CALL(glEnableVertexAttribArray(index));

            if (is_int) {
                GL_CALL(glVertexAttribIPointer(index, size, type, 0, (void*)offset));
            }
            else {
                GL_CALL(glVertexAttribPointer(index, size, type, GL_FALSE, 0, (void*)offset));
            }
            offset += size * (is_int ? sizeof(int) : sizeof(float));
        }
    }
}
}