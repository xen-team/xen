#pragma once

#include <utils/shape.hpp>

namespace xen {
class Entity;

/// [Bounding Volume Hierarchy](https://en.wikipedia.org/wiki/Bounding_volume_hierarchy) (BVH) node, holding the
/// necessary information
///  to perform queries on the BVH.
/// \see BoundingVolumeHierarchy
class BoundingVolumeHierarchyNode {
    friend class BoundingVolumeHierarchy;

public:
    BoundingVolumeHierarchyNode() = default;
    BoundingVolumeHierarchyNode(BoundingVolumeHierarchyNode const&) = delete;
    BoundingVolumeHierarchyNode(BoundingVolumeHierarchyNode&&) noexcept = default;

    BoundingVolumeHierarchyNode& operator=(BoundingVolumeHierarchyNode const&) = delete;
    BoundingVolumeHierarchyNode& operator=(BoundingVolumeHierarchyNode&&) noexcept = default;

    AABB const& get_bounding_box() const { return bounding_box; }

    bool has_left_child() const { return (left_child != nullptr); }

    BoundingVolumeHierarchyNode const& get_left_child() const
    {
        Log::rt_assert(has_left_child());
        return *left_child;
    }

    bool has_right_child() const { return (right_child != nullptr); }

    BoundingVolumeHierarchyNode const& get_right_child() const
    {
        Log::rt_assert(has_right_child());
        return *right_child;
    }

    Triangle const& get_triangle() const { return triangle_info.triangle; }

    /// Checks if the current node is a leaf, that is, a node without any child.
    /// \note This is a requirement for the triangle information to be valid.
    /// \return True if it is a leaf node, false otherwise.
    bool is_leaf() const { return left_child == nullptr && right_child == nullptr; }

    /// Queries the BVH node to find the closest entity intersected by the given ray.
    /// \param ray Ray to query the BVH node with.
    /// \param hit Optional ray intersection's information to recover (nullptr if unneeded).
    /// \return Closest entity intersected.
    Entity* query(Ray const& ray, RayHit* hit = nullptr) const;

private:
    struct TriangleInfo {
        Triangle triangle = Triangle(
            Vector3f(0.f), Vector3f(0.f), Vector3f(0.f)
        );                ///< Triangle contained by the node. Only valid if the node is a leaf.
        Entity* entity{}; ///< Entity containing the triangle. Only valid if the node is a leaf.
    };

    AABB bounding_box = AABB(Vector3f(0.f), Vector3f(0.f));
    std::unique_ptr<BoundingVolumeHierarchyNode> left_child{};
    std::unique_ptr<BoundingVolumeHierarchyNode> right_child{};
    TriangleInfo triangle_info; ///< Triangle/entity pair. Only valid if the node is a leaf.

private:
    /// Builds the node and its children from a list of triangles.
    /// \param triangles_info List of triangles information to build the node from.
    /// \param begin_index First index in the triangles' list.
    /// \param end_index Past-the-end index in the triangles' list.
    void build(std::vector<TriangleInfo>& triangles_info, size_t begin_index, size_t end_index);
};

/// [Bounding Volume Hierarchy](https://en.wikipedia.org/wiki/Bounding_volume_hierarchy) (BVH) data structure, organized
/// as a binary tree. This can be used to perform efficient queries from a ray in the scene.
class BoundingVolumeHierarchy {
public:
    BoundingVolumeHierarchy() = default;
    BoundingVolumeHierarchy(BoundingVolumeHierarchy const&) = delete;
    BoundingVolumeHierarchy(BoundingVolumeHierarchy&&) noexcept = default;

    BoundingVolumeHierarchy& operator=(BoundingVolumeHierarchy const&) = delete;
    BoundingVolumeHierarchy& operator=(BoundingVolumeHierarchy&&) noexcept = default;

    BoundingVolumeHierarchyNode const& get_root_node() const { return root_node; }

    /// Builds the BVH from the given entities.
    /// \param entities Entities with which to build the BVH from. They must have a Mesh component in order to be used
    /// for the build.
    void build(std::vector<Entity*> const& entities);

    /// Queries the BVH to find the closest entity intersected by the given ray.
    /// \param ray Ray to query the BVH with.
    /// \param hit Optional ray intersection's information to recover (nullptr if unneeded).
    /// \return Closest entity intersected.
    Entity* query(Ray const& ray, RayHit* hit = nullptr) const { return root_node.query(ray, hit); }

private:
    BoundingVolumeHierarchyNode root_node{};
};
}