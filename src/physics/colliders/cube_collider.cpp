#include "cube_collider.hpp"
#include <BulletCollision/CollisionShapes/btBoxShape.h>

namespace xen {
CubeCollider::CubeCollider(Vector3f const& extents, TransformComponent const& local_transform) :
    // Collider(local_transform),
    shape(std::make_unique<btBoxShape>(convert(extents / 2.0f))), extents(extents)
{
    this->local_transform = local_transform;
    this->local_transform.scale = extents;
}

btCollisionShape* CubeCollider::get_collision_shape() const
{
    return shape.get();
}

void CubeCollider::set_extents(Vector3f const& extents)
{
    this->extents = extents;
    shape->setImplicitShapeDimensions(convert(extents));
    local_transform.scale = extents;
}

void to_json(json& j, CubeCollider const& p)
{
    to_json(j["local_transform"], p.local_transform);
    to_json(j["extents"], p.extents);
}

void from_json(json const& j, CubeCollider& p)
{
    from_json(j["local_transform"], p.local_transform);
    from_json(j["extents"], p.extents);
}
}