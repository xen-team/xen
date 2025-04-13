#include "cone_collider.hpp"
#include <BulletCollision/CollisionShapes/btConeShape.h>

namespace xen {
ConeCollider::ConeCollider(float const radius, float const height, TransformComponent const& local_transform) :
    // Collider(localTransform),
    shape(std::make_unique<btConeShape>(radius, height)), radius(radius), height(height)
{
    this->local_transform = local_transform;
    this->local_transform.scale = {radius, height, radius};
}

btCollisionShape* ConeCollider::get_collision_shape() const
{
    return shape.get();
}

void ConeCollider::set_radius(float const radius)
{
    this->radius = radius;
    shape->setRadius(radius);
    local_transform.scale = {radius, height, radius};
}

void ConeCollider::set_height(float const height)
{
    this->height = height;
    shape->setHeight(height);
    local_transform.scale = {radius, height, radius};
}

void to_json(json& j, ConeCollider const& p)
{
    to_json(j["local_transform"], p.local_transform);
    to_json(j["radius"], p.radius);
    to_json(j["height"], p.height);
}

void from_json(json const& j, ConeCollider& p)
{
    from_json(j["local_transform"], p.local_transform);
    from_json(j["radius"], p.radius);
    from_json(j["height"], p.height);
}
}