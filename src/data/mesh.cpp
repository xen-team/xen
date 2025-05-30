#include "mesh.hpp"

#include <utils/threading.hpp>

#include <tracy/Tracy.hpp>

namespace xen {
Mesh::Mesh(Plane const& plane, float width, float depth)
{
    ZoneScopedN("Mesh::Mesh(Plane)");

    float const height = plane.compute_centroid().y;

    // TODO: creating a Mesh from a Plane doesn't take the normal into account for the vertices' position
    Vector3f const first_pos(-width, height, depth);
    Vector3f const second_pos(width, height, depth);
    Vector3f const third_pos(width, height, -depth);
    Vector3f const fourth_pos(-width, height, -depth);

    Vertex first_corner{};
    first_corner.position = first_pos;
    first_corner.normal = plane.get_normal();
    first_corner.texcoords = Vector2f(0.f, 0.f);

    Vertex second_corner{};
    second_corner.position = second_pos;
    second_corner.normal = plane.get_normal();
    second_corner.texcoords = Vector2f(1.f, 0.f);

    Vertex third_corner{};
    third_corner.position = third_pos;
    third_corner.normal = plane.get_normal();
    third_corner.texcoords = Vector2f(1.f, 1.f);

    Vertex fourthCorner{};
    fourthCorner.position = fourth_pos;
    fourthCorner.normal = plane.get_normal();
    fourthCorner.texcoords = Vector2f(0.f, 1.f);

    Submesh& submesh = submeshes.emplace_back();

    submesh.get_vertices() = {first_corner, second_corner, third_corner, fourthCorner};
    submesh.get_triangle_indices() = {0, 1, 2, 0, 2, 3};

    compute_tangents();
}

Mesh::Mesh(Sphere const& sphere, uint32_t subdiv_count, SphereMeshType type)
{
    ZoneScopedN("Mesh::Mesh(Sphere)");

    if (subdiv_count < 1) {
        throw std::invalid_argument("Error: Cannot create a sphere mesh with no subdivision");
    }

    switch (type) {
    case SphereMeshType::UV:
        create_uv_sphere(sphere, Vector2ui(subdiv_count));
        break;

    case SphereMeshType::ICO:
        create_icosphere(sphere, subdiv_count);
        break;
    }

    compute_tangents();
}

Mesh::Mesh(
    Triangle const& triangle, Vector2f const& first_texcoords, Vector2f const& second_texcoords,
    Vector2f const& third_texcoords
)
{
    ZoneScopedN("Mesh::Mesh(Triangle)");

    Vector3f const& first_pos = triangle.get_first_pos();
    Vector3f const& second_pos = triangle.get_second_pos();
    Vector3f const& third_pos = triangle.get_third_pos();
    Vector3f const normal = triangle.compute_normal();

    Vertex first_vertex{};
    first_vertex.position = first_pos;
    first_vertex.texcoords = first_texcoords;
    first_vertex.normal = normal;

    Vertex second_vertex{};
    second_vertex.position = second_pos;
    second_vertex.texcoords = second_texcoords;
    second_vertex.normal = normal;

    Vertex third_vertex{};
    third_vertex.position = third_pos;
    third_vertex.texcoords = third_texcoords;
    third_vertex.normal = normal;

    Submesh& submesh = submeshes.emplace_back();

    submesh.get_vertices() = {first_vertex, second_vertex, third_vertex};
    submesh.get_triangle_indices() = {0, 1, 2};

    compute_tangents();
}

Mesh::Mesh(Quad const& quad)
{
    ZoneScopedN("Mesh::Mesh(Quad)");

    Vector3f const& left_top_pos = quad.get_top_left_pos();
    Vector3f const& right_top_pos = quad.get_top_right_pos();
    Vector3f const& right_bottom_pos = quad.get_bottom_right_pos();
    Vector3f const& left_bottom_pos = quad.get_bottom_left_pos();

    Vertex top_left{};
    top_left.position = left_top_pos;
    top_left.texcoords = Vector2f(0.f, 1.f);

    Vertex top_right{};
    top_right.position = right_top_pos;
    top_right.texcoords = Vector2f(1.f, 1.f);

    Vertex bottom_right{};
    bottom_right.position = right_bottom_pos;
    bottom_right.texcoords = Vector2f(1.f, 0.f);

    Vertex bottom_left{};
    bottom_left.position = left_bottom_pos;
    bottom_left.texcoords = Vector2f(0.f, 0.f);

    // Computing normals
    top_left.normal = (left_top_pos - right_top_pos).cross(left_bottom_pos - left_top_pos).normalize();
    top_right.normal = (right_top_pos - right_bottom_pos).cross(left_top_pos - right_top_pos).normalize();
    bottom_right.normal = (right_bottom_pos - left_bottom_pos).cross(right_top_pos - right_bottom_pos).normalize();
    bottom_left.normal = (left_bottom_pos - left_top_pos).cross(right_bottom_pos - left_bottom_pos).normalize();

    Submesh& submesh = submeshes.emplace_back();

    submesh.get_vertices() = {top_left, bottom_left, bottom_right, top_right};
    submesh.get_triangle_indices() = {0, 1, 2, 0, 2, 3};

    compute_tangents();
}

Mesh::Mesh(AABB const& box)
{
    ZoneScopedN("Mesh::Mesh(AABB)");

    auto const [min_x, min_y, min_z] = box.get_min_position();
    auto const [max_x, max_y, max_z] = box.get_max_position();

    Vector3f const top_rightBack(max_x, max_y, min_z);
    Vector3f const top_rightFront(max_x, max_y, max_z);
    Vector3f const bottom_rightBack(max_x, min_y, min_z);
    Vector3f const bottom_rightFront(max_x, min_y, max_z);
    Vector3f const top_leftBack(min_x, max_y, min_z);
    Vector3f const top_leftFront(min_x, max_y, max_z);
    Vector3f const bottom_leftBack(min_x, min_y, min_z);
    Vector3f const bottom_leftFront(min_x, min_y, max_z);

    Submesh& submesh = submeshes.emplace_back();

    std::vector<Vertex>& vertices = submesh.get_vertices();
    vertices.reserve(24);

    // Right face
    vertices.emplace_back(Vertex{bottom_rightFront, Vector2f(0.f, 0.f), Vector3f::Right, -Vector3f::Forward});
    vertices.emplace_back(Vertex{bottom_rightBack, Vector2f(1.f, 0.f), Vector3f::Right, -Vector3f::Forward});
    vertices.emplace_back(Vertex{top_rightFront, Vector2f(0.f, 1.f), Vector3f::Right, -Vector3f::Forward});
    vertices.emplace_back(Vertex{top_rightBack, Vector2f(1.f, 1.f), Vector3f::Right, -Vector3f::Forward});

    // Left face
    vertices.emplace_back(Vertex{bottom_leftBack, Vector2f(0.f, 0.f), -Vector3f::Right, Vector3f::Forward});
    vertices.emplace_back(Vertex{bottom_leftFront, Vector2f(1.f, 0.f), -Vector3f::Right, Vector3f::Forward});
    vertices.emplace_back(Vertex{top_leftBack, Vector2f(0.f, 1.f), -Vector3f::Right, Vector3f::Forward});
    vertices.emplace_back(Vertex{top_leftFront, Vector2f(1.f, 1.f), -Vector3f::Right, Vector3f::Forward});

    // Top face
    vertices.emplace_back(Vertex{top_leftFront, Vector2f(0.f, 0.f), Vector3f::Up, Vector3f::Right});
    vertices.emplace_back(Vertex{top_rightFront, Vector2f(1.f, 0.f), Vector3f::Up, Vector3f::Right});
    vertices.emplace_back(Vertex{top_leftBack, Vector2f(0.f, 1.f), Vector3f::Up, Vector3f::Right});
    vertices.emplace_back(Vertex{top_rightBack, Vector2f(1.f, 1.f), Vector3f::Up, Vector3f::Right});

    // Bottom face
    vertices.emplace_back(Vertex{bottom_leftBack, Vector2f(0.f, 0.f), -Vector3f::Up, Vector3f::Right});
    vertices.emplace_back(Vertex{bottom_rightBack, Vector2f(1.f, 0.f), -Vector3f::Up, Vector3f::Right});
    vertices.emplace_back(Vertex{bottom_leftFront, Vector2f(0.f, 1.f), -Vector3f::Up, Vector3f::Right});
    vertices.emplace_back(Vertex{bottom_rightFront, Vector2f(1.f, 1.f), -Vector3f::Up, Vector3f::Right});

    // Front face
    vertices.emplace_back(Vertex{bottom_leftFront, Vector2f(0.f, 0.f), Vector3f::Forward, Vector3f::Right});
    vertices.emplace_back(Vertex{bottom_rightFront, Vector2f(1.f, 0.f), Vector3f::Forward, Vector3f::Right});
    vertices.emplace_back(Vertex{top_leftFront, Vector2f(0.f, 1.f), Vector3f::Forward, Vector3f::Right});
    vertices.emplace_back(Vertex{top_rightFront, Vector2f(1.f, 1.f), Vector3f::Forward, Vector3f::Right});

    // Back face
    vertices.emplace_back(Vertex{bottom_rightBack, Vector2f(0.f, 0.f), -Vector3f::Forward, -Vector3f::Right});
    vertices.emplace_back(Vertex{bottom_leftBack, Vector2f(1.f, 0.f), -Vector3f::Forward, -Vector3f::Right});
    vertices.emplace_back(Vertex{top_rightBack, Vector2f(0.f, 1.f), -Vector3f::Forward, -Vector3f::Right});
    vertices.emplace_back(Vertex{top_leftBack, Vector2f(1.f, 1.f), -Vector3f::Forward, -Vector3f::Right});

    submesh.get_triangle_indices() = {
        0,  1,  2,  1,  3,  2,  // Right face
        4,  5,  6,  5,  7,  6,  // Left face
        8,  9,  10, 9,  11, 10, // Top face
        12, 13, 14, 13, 15, 14, // Bottom face
        16, 17, 18, 17, 19, 18, // Front face
        20, 21, 22, 21, 23, 22  // Back face
    };
}

size_t Mesh::recover_vertex_count() const
{
    size_t vertex_count = 0;

    for (Submesh const& submesh : submeshes) {
        vertex_count += submesh.get_vertex_count();
    }

    return vertex_count;
}

size_t Mesh::recover_triangle_count() const
{
    size_t index_count = 0;

    for (Submesh const& submesh : submeshes) {
        index_count += submesh.get_triangle_index_count();
    }

    return index_count / 3;
}

AABB const& Mesh::compute_bounding_box()
{
    ZoneScopedN("Mesh::compute_bounding_box");

    Vector3f min_pos(std::numeric_limits<float>::max());
    Vector3f max_pos(std::numeric_limits<float>::lowest());

    for (Submesh& submesh : submeshes) {
        AABB const& bounding_box = submesh.compute_bounding_box();

        min_pos.x = std::min(min_pos.x, bounding_box.get_min_position().x);
        min_pos.y = std::min(min_pos.y, bounding_box.get_min_position().y);
        min_pos.z = std::min(min_pos.z, bounding_box.get_min_position().z);

        max_pos.x = std::max(max_pos.x, bounding_box.get_max_position().x);
        max_pos.y = std::max(max_pos.y, bounding_box.get_max_position().y);
        max_pos.z = std::max(max_pos.z, bounding_box.get_max_position().z);
    }

    bounding_box = AABB(min_pos, max_pos);
    return bounding_box;
}

void Mesh::compute_tangents()
{
    ZoneScopedN("Mesh::compute_tangents");

    if (submeshes.empty()) {
        return;
    }

    parallelize(submeshes, [](auto const& range) {
        for (Submesh& submesh : range) {
            submesh.compute_tangents();
        }
    });
}

Mesh Mesh::clone() const
{
    Mesh mesh;
    mesh.submeshes.reserve(submeshes.size());
    for (auto const& submesh : submeshes) {
        mesh.submeshes.emplace_back(submesh.clone());   
    }
    mesh.bounding_box = bounding_box;
    return mesh;
}

void Mesh::create_uv_sphere(Sphere const& sphere, Vector2ui const& lines_count)
{
    // Algorithm based on the standard/UV sphere presented here: http://www.songho.ca/opengl/gl_sphere.html#sphere

    ZoneScopedN("Mesh::create_uv_sphere");

    Submesh& submesh = submeshes.emplace_back();

    std::vector<Vertex>& vertices = submesh.get_vertices();
    vertices.reserve((lines_count.y + 1) * (lines_count.x + 1));

    float const width_step = 2 * std::numbers::pi_v<float> / static_cast<float>(lines_count.x);
    float const height_step = std::numbers::pi_v<float> / static_cast<float>(lines_count.y);
    Vector3f const center = sphere.get_center();

    for (uint height_index = 0; height_index <= lines_count.y; ++height_index) {
        float const height_angle = std::numbers::pi_v<float> / 2 - static_cast<float>(height_index) * height_step;

        float const xz = sphere.get_radius() * std::cos(height_angle);
        float const y = sphere.get_radius() * std::sin(height_angle);

        for (uint width_index = 0; width_index <= lines_count.x; ++width_index) {
            float const width_angle = static_cast<float>(width_index) * width_step;

            float const x = xz * std::cos(width_angle);
            float const z = xz * std::sin(width_angle);

            Vertex vert;
            vert.position = Vector3f(x + center.x, y + center.y, z + center.z);
            vert.texcoords = Vector2f(
                static_cast<float>(width_index) / static_cast<float>(lines_count.x),
                static_cast<float>(height_index) / static_cast<float>(lines_count.y)
            );
            vert.normal = Vector3f(x, y, z).normalize(
            ); // Dividing by the inverse radius does not give a perfectly unit vector; normalizing directly

            vertices.emplace_back(vert);
        }
    }

    std::vector<uint>& indices = submesh.get_triangle_indices();
    indices.reserve(lines_count.x * 6 + (lines_count.y - 2) * lines_count.x * 6);

    // Upper circle
    for (uint width_index = 0; width_index < lines_count.x; ++width_index) {
        uint const width_stride = lines_count.x + width_index;

        indices.push_back(width_stride + 1);
        indices.push_back(width_index + 1);
        indices.push_back(width_stride + 2);
    }

    for (uint height_index = 1; height_index < lines_count.y - 1; ++height_index) {
        uint cur_height_stride = height_index * (lines_count.x + 1);
        uint next_height_stride = cur_height_stride + lines_count.x + 1;

        for (uint width_index = 0; width_index < lines_count.x;
             ++width_index, ++cur_height_stride, ++next_height_stride) {
            indices.push_back(next_height_stride);
            indices.push_back(cur_height_stride);
            indices.push_back(cur_height_stride + 1);

            indices.push_back(next_height_stride);
            indices.push_back(cur_height_stride + 1);
            indices.push_back(next_height_stride + 1);
        }
    }

    // Lower circle
    {
        uint cur_height_stride = (lines_count.y - 1) * (lines_count.x + 1);
        uint next_height_stride = cur_height_stride + lines_count.x + 1;

        for (uint width_index = 0; width_index < lines_count.x;
             ++width_index, ++cur_height_stride, ++next_height_stride) {
            indices.push_back(next_height_stride);
            indices.push_back(cur_height_stride);
            indices.push_back(cur_height_stride + 1);
        }
    }
}

void Mesh::create_icosphere(Sphere const& sphere, uint32_t /* subdiv_count */)
{
    // Algorithm based on the icosphere presented here:
    // - http://www.songho.ca/opengl/gl_sphere.html#icosphere
    // - https://gist.github.com/warmwaffles/402b9c04318d6ee6dfa4

    ZoneScopedN("Mesh::create_icosphere");

    float const radius = sphere.get_radius();
    float const golden_radius = radius * Math::GoldenRatio<float>;

    Submesh& submesh = submeshes.emplace_back();

    std::vector<Vertex>& vertices = submesh.get_vertices();
    vertices.resize(12);

    constexpr float inv_factor = 1.f / (std::numbers::pi_v<float> * 2);

    vertices[0].normal = Vector3f(-radius, golden_radius, 0.f).normalize();
    vertices[0].position = vertices[0].normal * radius;
    vertices[0].texcoords = Vector2f(
        std::atan2(vertices[0].normal[0], vertices[0].normal[2]) * inv_factor + 0.5f,
        vertices[0].normal[1] * 0.5f + 0.5f
    );

    vertices[1].normal = Vector3f(radius, golden_radius, 0.f).normalize();
    vertices[1].position = vertices[1].normal * radius;
    vertices[1].texcoords = Vector2f(
        std::atan2(vertices[1].normal[0], vertices[1].normal[2]) * inv_factor + 0.5f,
        vertices[1].normal[1] * 0.5f + 0.5f
    );

    vertices[2].normal = Vector3f(-radius, -golden_radius, 0.f).normalize();
    vertices[2].position = vertices[2].normal * radius;
    vertices[2].texcoords = Vector2f(
        std::atan2(vertices[2].normal[0], vertices[2].normal[2]) * inv_factor + 0.5f,
        vertices[2].normal[1] * 0.5f + 0.5f
    );

    vertices[3].normal = Vector3f(radius, -golden_radius, 0.f).normalize();
    vertices[3].position = vertices[3].normal * radius;
    vertices[3].texcoords = Vector2f(
        std::atan2(vertices[3].normal[0], vertices[3].normal[2]) * inv_factor + 0.5f,
        vertices[3].normal[1] * 0.5f + 0.5f
    );

    vertices[4].normal = Vector3f(0.f, -radius, golden_radius).normalize();
    vertices[4].position = vertices[4].normal * radius;
    vertices[4].texcoords = Vector2f(
        std::atan2(vertices[4].normal[0], vertices[4].normal[2]) * inv_factor + 0.5f,
        vertices[4].normal[1] * 0.5f + 0.5f
    );

    vertices[5].normal = Vector3f(0.f, radius, golden_radius).normalize();
    vertices[5].position = vertices[5].normal * radius;
    vertices[5].texcoords = Vector2f(
        std::atan2(vertices[5].normal[0], vertices[5].normal[2]) * inv_factor + 0.5f,
        vertices[5].normal[1] * 0.5f + 0.5f
    );

    vertices[6].normal = Vector3f(0.f, -radius, -golden_radius).normalize();
    vertices[6].position = vertices[6].normal * radius;
    vertices[6].texcoords = Vector2f(
        std::atan2(vertices[6].normal[0], vertices[6].normal[2]) * inv_factor + 0.5f,
        vertices[6].normal[1] * 0.5f + 0.5f
    );

    vertices[7].normal = Vector3f(0.f, radius, -golden_radius).normalize();
    vertices[7].position = vertices[7].normal * radius;
    vertices[7].texcoords = Vector2f(
        std::atan2(vertices[7].normal[0], vertices[7].normal[2]) * inv_factor + 0.5f,
        vertices[7].normal[1] * 0.5f + 0.5f
    );

    vertices[8].normal = Vector3f(golden_radius, 0.f, -radius).normalize();
    vertices[8].position = vertices[8].normal * radius;
    vertices[8].texcoords = Vector2f(
        std::atan2(vertices[8].normal[0], vertices[8].normal[2]) * inv_factor + 0.5f,
        vertices[8].normal[1] * 0.5f + 0.5f
    );

    vertices[9].normal = Vector3f(golden_radius, 0.f, radius).normalize();
    vertices[9].position = vertices[9].normal * radius;
    vertices[9].texcoords = Vector2f(
        std::atan2(vertices[9].normal[0], vertices[9].normal[2]) * inv_factor + 0.5f,
        vertices[9].normal[1] * 0.5f + 0.5f
    );

    vertices[10].normal = Vector3f(-golden_radius, 0.f, -radius).normalize();
    vertices[10].position = vertices[10].normal * radius;
    vertices[10].texcoords = Vector2f(
        std::atan2(vertices[10].normal[0], vertices[10].normal[2]) * inv_factor + 0.5f,
        vertices[10].normal[1] * 0.5f + 0.5f
    );

    vertices[11].normal = Vector3f(-golden_radius, 0.f, radius).normalize();
    vertices[11].position = vertices[11].normal * radius;
    vertices[11].texcoords = Vector2f(
        std::atan2(vertices[11].normal[0], vertices[11].normal[2]) * inv_factor + 0.5f,
        vertices[11].normal[1] * 0.5f + 0.5f
    );

    // Applying a translation on each vertex by the sphere's center
    for (Vertex& vertex : vertices) {
        vertex.position += sphere.get_center();
    }

    submesh.get_triangle_indices() = {5,  0, 11, 1,  0, 5,  7, 0, 1,  10, 0, 7,  11, 0, 10, 9, 1, 5, 4, 5,
                                      11, 2, 11, 10, 6, 10, 7, 8, 7,  1,  4, 3,  9,  2, 3,  4, 6, 3, 2, 8,
                                      3,  6, 9,  3,  8, 5,  4, 9, 11, 2,  4, 10, 6,  2, 7,  8, 6, 1, 9, 8};

    // Applying subdivisions

    // TODO: subdivisions are clearly broken for the moment, must be investigated

    //  std::unordered_map<Vertex, size_t> verticesIndices;
    //  verticesIndices.emplace(vertices[0], 0);
    //  verticesIndices.emplace(vertices[1], 1);
    //  verticesIndices.emplace(vertices[2], 2);
    //  verticesIndices.emplace(vertices[3], 3);
    //  verticesIndices.emplace(vertices[4], 4);
    //  verticesIndices.emplace(vertices[5], 5);
    //  verticesIndices.emplace(vertices[6], 6);
    //  verticesIndices.emplace(vertices[7], 7);
    //  verticesIndices.emplace(vertices[8], 8);
    //  verticesIndices.emplace(vertices[9], 9);
    //  verticesIndices.emplace(vertices[10], 10);
    //  verticesIndices.emplace(vertices[11], 11);
    //
    //  for (uint32_t subdivIndex = 0; subdivIndex < subdiv_count; ++subdivIndex) {
    //    std::vector<Vertex> newVertices;
    //    newVertices.reserve(vertices.size() * 2);
    //
    //    std::vector<uint> newIndices;
    //    newIndices.reserve(indices.size() * 9);
    //
    //    const size_t index_count = indices.size();
    //
    //    for (size_t triangleIndex = 0; triangleIndex < index_count; triangleIndex += 3) {
    //      // Recovering the original vertices
    //      //       3
    //      //      / \
//      //     /   \
//      //    /     \
//      //   /       \
//      //  /         \
//      // 1-----------2
    //
    //      const Vertex& first_vertex  = vertices[indices[triangleIndex]];
    //      const Vertex& second_vertex = vertices[indices[triangleIndex + 1]];
    //      const Vertex& third_vertex  = vertices[indices[triangleIndex + 2]];
    //
    //      newVertices.emplace_back(first_vertex);
    //      newVertices.emplace_back(second_vertex);
    //      newVertices.emplace_back(third_vertex);
    //
    //      // Computing the edge vertices to form another triangle
    //      //       x
    //      //      / \
//      //     /   \
//      //    3-----2
    //      //   / \   / \
//      //  /   \ /   \
//      // x-----1-----x
    //
    //      Vertex& first_edgeVert   = newVertices.emplace_back();
    //      first_edgeVert.position  = (first_vertex.position + second_vertex.position) * 0.5f;
    //      first_edgeVert.texcoords = (first_vertex.texcoords + second_vertex.texcoords) * 0.5f;
    //      first_edgeVert.normal    = (first_vertex.normal + second_vertex.normal).normalize();
    //
    //      Vertex& second_edgeVert   = newVertices.emplace_back();
    //      second_edgeVert.position  = (second_vertex.position + third_vertex.position) * 0.5f;
    //      second_edgeVert.texcoords = (second_vertex.texcoords + third_vertex.texcoords) * 0.5f;
    //      second_edgeVert.normal    = (second_vertex.normal + third_vertex.normal).normalize();
    //
    //      Vertex& thirdEdgeVert   = newVertices.emplace_back();
    //      thirdEdgeVert.position  = (third_vertex.position + first_vertex.position) * 0.5f;
    //      thirdEdgeVert.texcoords = (third_vertex.texcoords + first_vertex.texcoords) * 0.5f;
    //      thirdEdgeVert.normal    = (third_vertex.normal + first_vertex.normal).normalize();
    //
    //      // Adding indices to create the 4 resulting triangles
    //      //       x
    //      //      / \
//      //     / 4 \
//      //    x-----x
    //      //   / \ 1 / \
//      //  / 2 \ / 3 \
//      // x-----x-----x
    //
    //      const size_t first_edgeVertIndex = newVertices.size() - 3;
    //      const size_t second_edgeVertIndex = newVertices.size() - 2;
    //      const size_t thirdEdgeVertIndex = newVertices.size() - 1;
    //
    //      verticesIndices.emplace(first_edgeVert, first_edgeVertIndex);
    //      verticesIndices.emplace(second_edgeVert, second_edgeVertIndex);
    //      verticesIndices.emplace(thirdEdgeVert, thirdEdgeVertIndex);
    //
    //      newIndices.emplace_back(first_edgeVertIndex);
    //      newIndices.emplace_back(second_edgeVertIndex);
    //      newIndices.emplace_back(thirdEdgeVertIndex);
    //
    //      newIndices.emplace_back(verticesIndices.find(first_vertex)->second);
    //      newIndices.emplace_back(first_edgeVertIndex);
    //      newIndices.emplace_back(thirdEdgeVertIndex);
    //
    //      newIndices.emplace_back(first_edgeVertIndex);
    //      newIndices.emplace_back(verticesIndices.find(second_vertex)->second);
    //      newIndices.emplace_back(second_edgeVertIndex);
    //
    //      newIndices.emplace_back(thirdEdgeVertIndex);
    //      newIndices.emplace_back(second_edgeVertIndex);
    //      newIndices.emplace_back(verticesIndices.find(third_vertex)->second);
    //    }
    //
    //    vertices = std::move(newVertices);
    //    indices  = std::move(newIndices);
    //  }
}
}