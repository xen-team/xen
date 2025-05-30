#include "capsule_collider.hpp"
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>

namespace xen {
CapsuleCollider::CapsuleCollider(float radius, float height, Transform const& local_transform) :
    shape(std::make_unique<btCapsuleShape>(radius, height)), radius(radius), height(height)
{
    this->local_transform = local_transform;
    this->local_transform.set_scale({this->radius, this->height, this->radius});
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
    local_transform.set_scale({radius, height, radius});
}

void CapsuleCollider::set_height(float const height)
{
    this->height = height;
    shape->setImplicitShapeDimensions({radius, 0.5f * height, radius});
    local_transform.set_scale({radius, height, radius});
}
}