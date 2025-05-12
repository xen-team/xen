#include "submesh.hpp"

#include <tracy/Tracy.hpp>

namespace xen {
namespace {
constexpr Vector3f compute_tangent(Vertex const& first_vertex, Vertex const& second_vertex, Vertex const& third_vertex)
{
    Vector3f const first_edge = second_vertex.position - first_vertex.position;
    Vector3f const second_edge = third_vertex.position - first_vertex.position;

    Vector2f const first_uv_diff = second_vertex.texcoords - first_vertex.texcoords;
    Vector2f const second_uv_diff = third_vertex.texcoords - first_vertex.texcoords;

    float const denominator = (first_uv_diff.x * second_uv_diff.y - second_uv_diff.x * first_uv_diff.y);

    if (denominator == 0.f) {
        return Vector3f(0.f);
    }

    return (first_edge * second_uv_diff.y - second_edge * first_uv_diff.y) / denominator;
}
}

AABB const& Submesh::conpute_bounding_box()
{
    ZoneScopedN("Submesh::conpute_bounding_box");

    Vector3f min_pos(std::numeric_limits<float>::max());
    Vector3f max_pos(std::numeric_limits<float>::lowest());

    for (Vertex const& vert : vertices) {
        min_pos.x = std::min(min_pos.x, vert.position.x);
        min_pos.y = std::min(min_pos.y, vert.position.y);
        min_pos.z = std::min(min_pos.z, vert.position.z);

        max_pos.x = std::max(max_pos.x, vert.position.x);
        max_pos.y = std::max(max_pos.y, vert.position.y);
        max_pos.z = std::max(max_pos.z, vert.position.z);
    }

    bounding_box = AABB(min_pos, max_pos);
    return bounding_box;
}

void Submesh::compute_tangents()
{
    ZoneScopedN("Submesh::compute_tangents");

    for (Vertex& vert : vertices) {
        vert.tangent = Vector3f(0.f, 0.f, 0.f);
    }

    for (size_t i = 0; i < triangle_indices.size(); i += 3) {
        Vertex& first_vertex = vertices[triangle_indices[i]];
        Vertex& second_vertex = vertices[triangle_indices[i + 1]];
        Vertex& third_vertex = vertices[triangle_indices[i + 2]];

        Vector3f const tangent = compute_tangent(first_vertex, second_vertex, third_vertex);

        // Adding the computed tangent to each vertex; they will be normalized later
        first_vertex.tangent += tangent;
        second_vertex.tangent += tangent;
        third_vertex.tangent += tangent;
    }

    // Normalizing the accumulated tangents
    for (Vertex& vert : vertices) {
        // Avoiding NaNs by preventing the normalization of a 0 vector
        if (vert.tangent == Vector3f(0.f)) {
            continue;
        }

        vert.tangent = (vert.tangent - vert.normal * vert.tangent.dot(vert.normal)).normalize();
    }
}

}
