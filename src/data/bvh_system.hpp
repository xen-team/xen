#pragma once

#include <system.hpp>
#include <data/bvh.hpp>

namespace xen {
/// System dedicated to managing a [Bounding Volume Hierarchy](https://en.wikipedia.org/wiki/Bounding_volume_hierarchy)
/// (BVH) of the scene,
///  automatically updating it from linked and unlinked entities.
/// \see BoundingVolumeHierarchy
class BoundingVolumeHierarchySystem final : public System {
public:
    /// Default constructor.
    BoundingVolumeHierarchySystem();

    BoundingVolumeHierarchy const& get_bvh() const { return bvh; }

private:
    BoundingVolumeHierarchy bvh{};

private:
    /// Links the entity to the system and rebuilds the BVH.
    /// \param entity Entity to be linked.
    void link_entity(EntityPtr const& entity) override;

    /// Uninks the entity to the system and rebuilds the BVH.
    /// \param entity Entity to be unlinked.
    void unlink_entity(EntityPtr const& entity) override;
};
}