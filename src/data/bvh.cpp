#include "bvh.hpp"

#include <entity.hpp>
#include <data/mesh.hpp>
#include <math/transform/transform.hpp>

#include <tracy/Tracy.hpp>

namespace {
enum CutAxis : uint8_t { AXIS_X = 0, AXIS_Y = 1, AXIS_Z = 2 };
}

namespace xen {
Entity* BoundingVolumeHierarchyNode::query(Ray const& ray, RayHit* hit) const
{
    // The following call can produce way too many zones, *drastically* increasing the profiling time & memory
    // consumption
    // ZoneScopedN("BoundingVolumeHierarchyNode::query");

    if (is_leaf()) {
        if (ray.intersects(triangle_info.triangle, hit)) {
            return triangle_info.entity;
        }

        return nullptr;
    }

    if (!ray.intersects(bounding_box)) {
        return nullptr;
    }

    RayHit left_hit;
    RayHit right_hit;
    Entity* left_entity = (left_child != nullptr ? left_child->query(ray, &left_hit) : nullptr);
    Entity* right_entity = (right_child != nullptr ? right_child->query(ray, &right_hit) : nullptr);

    bool const is_left_closer = (left_hit.distance < right_hit.distance);

    if (hit) {
        *hit = (is_left_closer ? left_hit : right_hit);
    }

    // If both entities have been hit, return the closest one
    if (left_entity && right_entity) {
        return (is_left_closer ? left_entity : right_entity);
    }

    // Otherwise, return whichever has been hit, or none
    return (left_entity != nullptr ? left_entity : right_entity);
}

void BoundingVolumeHierarchyNode::build(std::vector<TriangleInfo>& triangles_info, size_t begin_index, size_t end_index)
{
    // The following call can produce way too many zones, *drastically* increasing the profiling time & memory
    // consumption
    // ZoneScopedN("BoundingVolumeHierarchyNode::build");

    bounding_box = triangles_info[begin_index].triangle.compute_bounding_box();

    if (end_index - begin_index <= 1) {
        triangle_info = triangles_info[begin_index];
        return;
    }

    // TODO: wait for a parallel/reduce to be implemented in order to optimize the following loop
    for (size_t i = begin_index + 1; i < end_index; ++i) {
        AABB const triangle_box = triangles_info[i].triangle.compute_bounding_box();

        float const x_min = std::min(triangle_box.get_min_position().x, bounding_box.get_min_position().x);
        float const y_min = std::min(triangle_box.get_min_position().y, bounding_box.get_min_position().y);
        float const z_min = std::min(triangle_box.get_min_position().z, bounding_box.get_min_position().z);

        float const x_max = std::max(triangle_box.get_max_position().x, bounding_box.get_max_position().x);
        float const y_max = std::max(triangle_box.get_max_position().y, bounding_box.get_max_position().y);
        float const z_max = std::max(triangle_box.get_max_position().z, bounding_box.get_max_position().z);

        bounding_box = AABB(Vector3f(x_min, y_min, z_min), Vector3f(x_max, y_max, z_max));
    }

    float max_length = bounding_box.get_max_position().x - bounding_box.get_min_position().x;
    CutAxis cut_axis = AXIS_X;

    float const max_y_length = bounding_box.get_max_position().y - bounding_box.get_min_position().y;
    if (max_y_length > max_length) {
        max_length = max_y_length;
        cut_axis = AXIS_Y;
    }

    float const max_z_length = bounding_box.get_max_position().z - bounding_box.get_min_position().z;
    if (max_z_length > max_length) {
        max_length = max_z_length;
        cut_axis = AXIS_Z;
    }

    // Reorganizing triangles by splitting them over the cut axis, according to their centroid
    float const half_cut_pos = bounding_box.get_min_position()[cut_axis] + (max_length * 0.5f);
    auto const mid_shape_iter = std::partition(
        triangles_info.begin() + static_cast<std::ptrdiff_t>(begin_index),
        triangles_info.begin() + static_cast<std::ptrdiff_t>(end_index),
        [cut_axis, half_cut_pos](TriangleInfo const& triangle_info) {
            return (triangle_info.triangle.compute_centroid()[cut_axis] < half_cut_pos);
        }
    );

    auto mid_index = static_cast<size_t>(std::distance(triangles_info.begin(), mid_shape_iter));
    if (mid_index == begin_index || mid_index == end_index) {
        mid_index = (begin_index + end_index) / 2;
    }

    left_child = std::make_unique<BoundingVolumeHierarchyNode>();
    left_child->build(triangles_info, begin_index, mid_index);

    right_child = std::make_unique<BoundingVolumeHierarchyNode>();
    right_child->build(triangles_info, mid_index, end_index);
}

void BoundingVolumeHierarchy::build(std::vector<Entity*> const& entities)
{
    ZoneScopedN("BoundingVolumeHierarchy::build");

    root_node = BoundingVolumeHierarchyNode();

    // Storing all triangles in a list to build the BVH from

    size_t total_triangle_count = 0;

    for (Entity const* entity : entities) {
        if (!entity->is_enabled() || !entity->has_component<Mesh>()) {
            continue;
        }

        total_triangle_count += entity->get_component<Mesh>().recover_triangle_count();
    }

    if (total_triangle_count == 0) {
        return; // No triangle to build the BVH from
    }

    std::vector<BoundingVolumeHierarchyNode::TriangleInfo> triangles;
    triangles.reserve(total_triangle_count);

    for (Entity* entity : entities) {
        if (!entity->is_enabled() || !entity->has_component<Mesh>()) {
            continue;
        }

        bool const has_transform = entity->has_component<Transform>();
        Matrix4 const transformation =
            (has_transform ? entity->get_component<Transform>().compute_transform() : Matrix4());

        for (Submesh const& submesh : entity->get_component<Mesh>().get_submeshes()) {
            for (size_t i = 0; i < submesh.get_triangle_index_count(); i += 3) {
                Triangle triangle(
                    submesh.get_vertices()[submesh.get_triangle_indices()[i]].position,
                    submesh.get_vertices()[submesh.get_triangle_indices()[i + 1]].position,
                    submesh.get_vertices()[submesh.get_triangle_indices()[i + 2]].position
                );

                if (has_transform) {
                    auto first_triangle_mat = transformation * Vector4f(triangle.get_first_pos(), 1.f);
                    auto second_triangle_mat = transformation * Vector4f(triangle.get_second_pos(), 1.f);
                    auto third_triangle_mat = transformation * Vector4f(triangle.get_third_pos(), 1.f);

                    triangle = Triangle(
                        Vector3f(first_triangle_mat[0][0], first_triangle_mat[0][1], first_triangle_mat[0][2]),
                        Vector3f(second_triangle_mat[0][0], second_triangle_mat[0][1], second_triangle_mat[0][2]),
                        Vector3f(third_triangle_mat[0][0], third_triangle_mat[0][1], third_triangle_mat[0][2])
                    );
                }

                triangles.emplace_back(BoundingVolumeHierarchyNode::TriangleInfo{triangle, entity});
            }
        }
    }

    root_node.build(triangles, 0, total_triangle_count);
}
}