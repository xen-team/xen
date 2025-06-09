#pragma once

#include <component.hpp>
#include <data/submesh.hpp>
#include <utils/shape.hpp>

namespace xen {
class FilePath;

enum class SphereMeshType {
    UV = 0, ///< [UV sphere](https://en.wikipedia.org/wiki/UV_mapping).
    ICO     ///< [Icosphere/convex icosahedron](https://en.wikipedia.org/wiki/Geodesic_polyhedron).
};

class Mesh : public Component {
public:
    Mesh() = default;
    Mesh(Mesh const&) = delete;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh const&) = delete;
    Mesh& operator=(Mesh&&) noexcept = default;

    Mesh(Plane const& plane, float width, float depth);

    /// Creates a mesh from a Sphere.
    /// \param sphere Sphere to create the mesh with.
    /// \param subdiv_count Amount of subdivisions (for a UV sphere, represents both the amount of vertical & horizontal
    /// lines to be created).
    /// \param type Type of the sphere mesh to create.
    Mesh(Sphere const& sphere, uint32_t subdiv_count, SphereMeshType type);

    Mesh(
        Triangle const& triangle, Vector2f const& first_texcoords, Vector2f const& second_texcoords,
        Vector2f const& third_texcoords
    );

    explicit Mesh(Quad const& quad);

    explicit Mesh(AABB const& box);

    std::vector<Submesh> const& get_submeshes() const { return submeshes; }

    std::vector<Submesh>& get_submeshes() { return submeshes; }

    AABB const& get_bounding_box() const { return bounding_box; }

    size_t recover_vertex_count() const;

    size_t recover_triangle_count() const;

    template <typename... Args>
    Submesh& add_submesh(Args&&... args)
    {
        return submeshes.emplace_back(std::forward<Args>(args)...);
    }

    /// Computes & updates the mesh's bounding box by computing the submeshes' ones.
    /// \return Mesh's bounding box.
    AABB const& compute_bounding_box();

    void set_bounding_box(AABB const& aabb) { bounding_box = aabb; };

    /// Computes the tangents for each of the mesh's vertices.
    void compute_tangents();

    [[nodiscard]] Mesh clone() const;

private:
    std::vector<Submesh> submeshes{};
    AABB bounding_box = AABB(Vector3f(0.f), Vector3f(0.f));

private:
    /// Creates an UV sphere mesh from a Sphere.
    ///
    ///          /-----------\
    ///        / / / / | / \ / \
    ///      /-------------------\
    ///     |/ | / | / | / | / | /|
    ///     |---------------------| < latitude/height
    ///     |/ | / | / | / | / | /|
    ///      \-------------------/
    ///        \ / \ / | / / / /
    ///          \-----^-----/
    ///                |
    ///                longitude/width
    ///
    /// \param sphere Sphere to create the mesh with.
    /// \param width_count Amount of vertical lines to be created (longitude).
    /// \param height_count Amount of horizontal lines to be created (latitude).
    void create_uv_sphere(Sphere const& sphere, Vector2ui const& lines_count);

    /// Creates an icosphere mesh from a Sphere.
    /// \param sphere Sphere to create the mesh with.
    /// \param subdiv_count Amount of subdivisions to apply to the mesh.
    void create_icosphere(Sphere const& sphere, uint32_t subdiv_count);
};
}