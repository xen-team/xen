#include "collider.hpp"
#include <BulletCollision/CollisionShapes/btCollisionShape.h>

namespace xen {
Collider::Collider(TransformComponent const& local_transform) : local_transform(local_transform) {}

void Collider::set_local_transform(TransformComponent const& local_transform)
{
    this->local_transform = local_transform;
    on_transform_change(this, this->local_transform);
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

btTransform Collider::convert(TransformComponent const& transform)
{
    btQuaternion rotation;
    rotation.setEulerZYX(transform.rotation.y, transform.rotation.x, transform.rotation.z);

    btTransform worldTransform;
    worldTransform.setIdentity();
    worldTransform.setOrigin(convert(transform.position));
    worldTransform.setRotation(rotation);
    return worldTransform;
}

TransformComponent Collider::convert(btTransform const& transform, Vector3f const& scaling)
{
    auto const position = transform.getOrigin();
    float yaw, pitch, roll;
    transform.getBasis().getEulerYPR(yaw, pitch, roll);
    return {convert(position), {pitch, yaw, roll}, scaling};
}
}