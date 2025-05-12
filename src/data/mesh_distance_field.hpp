#pragma once

#include <utils/shape.hpp>

namespace xen {
class BoundingVolumeHierarchy;
class Image;

/// 3-dimensional structure of signed distances to the closest mesh geometry in a specific area. Distances inside a mesh
/// will be negative.
class MeshDistanceField {
public:
    /// Creates a mesh distance field.
    /// \param area Area inside which the distances will be computed.
    /// \param width Number of divisions along the width; must be equal to or greater than 2.
    /// \param height Number of divisions along the height; must be equal to or greater than 2.
    /// \param depth Number of divisions along the depth; must be equal to or greater than 2.
    MeshDistanceField(AABB const& area, Vector3ui const& size);

    float get_distance(Vector3ui const& pos) const;

    void set_bvh(BoundingVolumeHierarchy const& bvh) { this->bvh = &bvh; }

    /// Computes the distance field's values for each point within the grid.
    /// \param sample_count Number of directions to sample around each point; a higher count will result in a better
    /// definition.
    /// \note This requires a BVH to have been set.
    /// \see set_bvh()
    void compute(size_t sample_count);

    /// Recovers the distance field's values in a list of 2D floating-point images.
    /// \return Images of each slice of the field along the depth.
    std::vector<Image> recover_slices() const;

private:
    AABB area = AABB(Vector3f(0.f), Vector3f(0.f));
    Vector3ui size;
    std::vector<float> distance_field{};
    BoundingVolumeHierarchy const* bvh = nullptr;

private:
    constexpr size_t compute_index(Vector3ui const& size) const
    {
        Log::rt_assert(size < this->size, "Error: The given width index is invalid.");
        return size.z * this->size.y * this->size.x + size.y * this->size.x + size.x;
    }
};
}