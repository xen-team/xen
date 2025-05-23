#include "collider.hpp"
#include <BulletCollision/CollisionShapes/btCollisionShape.h>

namespace xen {
Collider::Collider(Transform const& local_transform) : local_transform(local_transform) {}

void Collider::set_local_transform(Transform const& local_transform)
{
    this->local_transform = local_transform;
}

btVector3 Collider::convert(Vector3f const& vector)
{
    return btVector3{vector.x, vector.y, vector.z};
}

Vector3f Collider::convert(btVector3 const& vector)
{
    return {vector.getX(), vector.getY(), vector.getZ()};
}

btQuaternion Collider::convert(Quaternion const& quaternion)
{
    return {quaternion.x, quaternion.y, quaternion.z, quaternion.w};
}

Quaternion Collider::convert(btQuaternion const& quaternion)
{
    return {quaternion.getX(), quaternion.getY(), quaternion.getZ(), quaternion.getW()};
}

btTransform Collider::convert(Transform const& transform)
{
    btTransform worldTransform;
    worldTransform.setIdentity();
    worldTransform.setOrigin(convert(transform.get_position()));
    worldTransform.setRotation(convert(transform.get_rotation()));
    return worldTransform;
}

Transform Collider::convert(btTransform const& transform, Vector3f const& scaling)
{
    return Transform(convert(transform.getOrigin()), convert(transform.getRotation()), scaling);
}
}