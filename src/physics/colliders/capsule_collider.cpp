#include "capsule_collider.hpp"
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>

namespace xen {
CapsuleCollider::CapsuleCollider(float const radius, float const height, TransformComponent const& local_transform) :
    shape(std::make_unique<btCapsuleShape>(radius, height)), radius(radius), height(height)
{
    this->local_transform = local_transform;
    this->local_transform.scale = {radius, height, radius};
}

CapsuleCollider::~CapsuleCollider()
{
    // TODO: Intentional memory leak, for some reason `btCapsuleShape *` can't be deleted.
    shape.reset();
}

btCollisionShape* CapsuleCollider::get_collision_shape() const
{
    return shape.get();
}

void CapsuleCollider::set_radius(float const radius)
{
    this->radius = radius;
    shape->setImplicitShapeDimensions({radius, 0.5f * height, radius});
    local_transform.scale = {radius, height, radius};
}

void CapsuleCollider::set_height(float const height)
{
    this->height = height;
    shape->setImplicitShapeDimensions({radius, 0.5f * height, radius});
    local_transform.scale = {radius, height, radius};
}

void to_json(json& j, CapsuleCollider const& p)
{
    to_json(j["local_transform"], p.local_transform);
    to_json(j["radius"], p.radius);
    to_json(j["height"], p.height);
}

void from_json(json const& j, CapsuleCollider& p)
{
    from_json(j["local_transform"], p.local_transform);
    from_json(j["radius"], p.radius);
    from_json(j["height"], p.height);
}
}