#include "sphere_collider.hpp"
#include "math/math_serialization.hpp"

#include <BulletCollision/CollisionShapes/btSphereShape.h>

namespace xen {
SphereCollider::SphereCollider(float const radius, TransformComponent const& local_transform) :
    // Collider(local_transform),
    shape(std::make_unique<btSphereShape>(radius)), radius(radius)
{
    this->local_transform = local_transform;
    this->local_transform.scale = {radius, radius, radius};
}

btCollisionShape* SphereCollider::get_collision_shape() const
{
    return shape.get();
}

void SphereCollider::set_radius(float const radius)
{
    this->radius = radius;
    shape->setUnscaledRadius(radius);
    local_transform.scale = {radius, radius, radius};
}

void to_json(json& j, SphereCollider const& p)
{
    to_json(j["local_transform"], p.local_transform);
    to_json(j["radius"], p.radius);
}

void from_json(json const& j, SphereCollider& p)
{
    from_json(j["local_transform"], p.local_transform);
    from_json(j["radius"], p.radius);
}
}