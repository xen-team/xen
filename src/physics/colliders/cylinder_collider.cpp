#include "cylinder_collider.hpp"
#include "math/math_serialization.hpp"

#include <BulletCollision/CollisionShapes/btCylinderShape.h>

namespace xen {
CylinderCollider::CylinderCollider(float const radius, float const height, TransformComponent const& local_transform) :
    // Collider(local_transform),
    shape(std::make_unique<btCylinderShape>(btVector3(radius, height / 2.0f, radius))), radius(radius), height(height)
{
    this->local_transform = local_transform;
    this->local_transform.scale = {radius, height, radius};
}

btCollisionShape* CylinderCollider::get_collision_shape() const
{
    return shape.get();
}

void CylinderCollider::set_radius(float const radius)
{
    this->radius = radius;
    shape->setImplicitShapeDimensions({radius, height / 2.0f, radius});
    local_transform.scale = {radius, height, radius};
}

void CylinderCollider::set_height(float const height)
{
    this->height = height;
    shape->setImplicitShapeDimensions({radius, height / 2.0f, radius});
    local_transform.scale = {radius, height, radius};
}

void to_json(json& j, CylinderCollider const& p)
{
    to_json(j["local_transform"], p.local_transform);
    to_json(j["radius"], p.radius);
    to_json(j["height"], p.height);
}

void from_json(json const& j, CylinderCollider& p)
{
    from_json(j["local_transform"], p.local_transform);
    from_json(j["radius"], p.radius);
    from_json(j["height"], p.height);
}
}