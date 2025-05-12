#include "bvh_system.hpp"
#include <data/mesh.hpp>

namespace xen {
BoundingVolumeHierarchySystem::BoundingVolumeHierarchySystem()
{
    register_components<Mesh>();
}

void BoundingVolumeHierarchySystem::link_entity(EntityPtr const& entity)
{
    System::link_entity(entity);
    bvh.build(entities); // TODO: if N entities are linked one after the other, the BVH will be rebuilt as many times
}

void BoundingVolumeHierarchySystem::unlink_entity(EntityPtr const& entity)
{
    System::unlink_entity(entity);
    bvh.build(entities); // TODO: if N entities are unlinked one after the other, the BVH will be rebuilt as many times
}
}