#include "cube_collider.hpp"
#include <BulletCollision/CollisionShapes/btBoxShape.h>

namespace xen {
CubeCollider::CubeCollider(Vector3f const& extents, Transform const& local_transform) :
    // Collider(local_transform),
    shape(std::make_unique<btBoxShape>(convert(extents / 2.0f))), extents(extents)
{
    this->local_transform = local_transform;
    this->local_transform.set_scale(extents);
}

btCollisionShape* CubeCollider::get_collision_shape() const
{
    return shape.get();
}

void CubeCollider::set_extents(Vector3f const& extents)
{
    this->extents = extents;
    shape->setImplicitShapeDimensions(convert(extents));
    local_transform.set_scale(extents);
}
}