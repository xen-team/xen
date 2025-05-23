#pragma once

#include "collider.hpp"
#include "data/mesh.hpp"

#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <tracy/Tracy.hpp>

namespace xen {

class TriangleMeshCollider : public Collider {
public:
    explicit TriangleMeshCollider(xen::Mesh const& xen_mesh, Transform const& local_transform = Transform()) :
        Collider(local_transform), triangle_mesh_interface_(std::make_unique<btTriangleMesh>())
    {
        ZoneScopedN("TriangleMeshCollider::Constructor");
        build_triangle_mesh_interface(xen_mesh);
    }

    ~TriangleMeshCollider() = default;

    btCollisionShape* get_collision_shape() const override
    {
        ZoneScopedN("TriangleMeshCollider::get_collision_shape");
        if (!managed_bullet_shape_ && triangle_mesh_interface_ && triangle_mesh_interface_->getNumTriangles() > 0) {
            managed_bullet_shape_ =
                std::make_unique<btBvhTriangleMeshShape>(triangle_mesh_interface_.get(), true, true);
        }
        else if (!managed_bullet_shape_) {
            // Log::warn("[TriangleMeshCollider] Cannot get shape: no triangles or shape not initialized.");
        }
        return managed_bullet_shape_.get();
    }

    btTriangleMesh* get_triangle_mesh_interface() const { return triangle_mesh_interface_.get(); }

private:
    void build_triangle_mesh_interface(xen::Mesh const& xen_mesh)
    {
        ZoneScopedN("TriangleMeshCollider::build_triangle_mesh_interface");
        bool has_triangles = false;
        for (auto const& submesh : xen_mesh.get_submeshes()) {
            auto const& vertices = submesh.get_vertices();
            auto const& indices = submesh.get_triangle_indices();

            if (vertices.empty() || indices.empty()) {
                // Log::vdebug("[TriangleMeshCollider] Submesh is empty, skipping.");
                continue;
            }
            if (indices.size() % 3 != 0) {
                // Log::warn("[TriangleMeshCollider] Submesh index count ({}) is not a multiple of 3.", indices.size());
            }

            for (size_t i = 0; (i + 2) < indices.size(); i += 3) {
                uint const idx0 = indices[i];
                uint const idx1 = indices[i + 1];
                uint const idx2 = indices[i + 2];

                if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size()) {
                    // Log::error("[TriangleMeshCollider] Vertex index out of bounds. Idx: ({},{},{}), Vertices: {}.
                    // Skipping triangle.", idx0, idx1, idx2, vertices.size());
                    continue;
                }

                btVector3 const v0 = Collider::convert(vertices[idx0].position);
                btVector3 const v1 = Collider::convert(vertices[idx1].position);
                btVector3 const v2 = Collider::convert(vertices[idx2].position);

                triangle_mesh_interface_->addTriangle(v0, v1, v2, false);
                has_triangles = true;
            }
        }
        if (!has_triangles) {
            // Log::warn("[TriangleMeshCollider] No valid triangles found in xen_mesh to build collision interface.");
        }
        else {
            // Log::vdebug("[TriangleMeshCollider] Built triangle mesh interface with {} triangles.",
            // triangle_mesh_interface_->getNumTriangles());
        }
    }

    std::unique_ptr<btTriangleMesh> triangle_mesh_interface_;

    mutable std::unique_ptr<btBvhTriangleMeshShape> managed_bullet_shape_;
};
}