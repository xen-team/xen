#pragma once

#include <utils/shape.hpp>

namespace xen {
struct Vertex {
    Vector3f position{};
    Vector2f texcoords{};
    Vector3f normal{};
    Vector3f tangent{};

    constexpr bool operator==(Vertex const& vert) const
    {
        return (position == vert.position) && (texcoords == vert.texcoords) && (normal == vert.normal) &&
               (tangent == vert.tangent);
    }
    constexpr bool operator!=(Vertex const& vert) const { return !(*this == vert); }
};

inline std::ostream& operator<<(std::ostream& stream, Vertex const& vert)
{
    stream << "{\n"
           << "  " << vert.position << '\n'
           << "  " << vert.texcoords << '\n'
           << "  " << vert.normal << '\n'
           << "  " << vert.tangent << '\n'
           << '}';
    return stream;
}

class Submesh {
public:
    Submesh() = default;
    Submesh(Submesh const&) = delete;
    Submesh(Submesh&&) noexcept = default;
    Submesh& operator=(Submesh const&) = delete;
    Submesh& operator=(Submesh&&) noexcept = default;

    std::vector<Vertex> const& get_vertices() const { return vertices; }

    std::vector<Vertex>& get_vertices() { return vertices; }

    size_t get_vertex_count() const { return vertices.size(); }

    std::vector<uint> const& get_line_indices() const { return line_indices; }

    std::vector<uint>& get_line_indices() { return line_indices; }

    size_t get_line_index_count() const { return line_indices.size(); }

    std::vector<uint> const& get_triangle_indices() const { return triangle_indices; }

    std::vector<uint>& get_triangle_indices() { return triangle_indices; }

    size_t get_triangle_index_count() const { return triangle_indices.size(); }

    AABB const& get_bounding_box() const { return bounding_box; }

    /// Computes & updates the submesh's bounding box.
    /// \return Submesh's bounding box.
    AABB const& compute_bounding_box();

    /// Computes the tangents for each of the submesh's vertices.
    void compute_tangents();

private:
    std::vector<Vertex> vertices{};
    std::vector<uint> line_indices{};
    std::vector<uint> triangle_indices{};

    AABB bounding_box = AABB(Vector3f(0.f), Vector3f(0.f));
};
}

/// Specialization of std::hash for Vertex.
template <>
struct std::hash<xen::Vertex> {
    /// Computes the hash of the given vertex.
    /// \param vert Vertex to compute the hash of.
    /// \return Vertex's hash value.
    constexpr size_t operator()(xen::Vertex const& vert) const
    {
        return vert.tangent.hash(vert.normal.hash(vert.texcoords.hash(vert.position.hash(0))));
    }
};