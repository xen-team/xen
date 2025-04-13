#include "convex_hull_collider.hpp"
#include "math/math_serialization.hpp"

#include <BulletCollision/CollisionShapes/btConvexHullShape.h>

namespace xen {
ConvexHullCollider::ConvexHullCollider(
    std::vector<float> const& point_cloud, TransformComponent const& local_transform
) /*:
Collider(local_transform)*/
{
    this->local_transform = local_transform;
    if (!point_cloud.empty()) {
        set_point_count(point_cloud);
    }
}

btCollisionShape* ConvexHullCollider::get_collision_shape() const
{
    return shape.get();
}

void ConvexHullCollider::set_point_count(std::vector<float> const& point_cloud)
{
    if (point_cloud.empty()) {
        return;
    }

    shape = std::make_unique<btConvexHullShape>(
        point_cloud.data(), static_cast<int32_t>(point_cloud.size() / 3), static_cast<int32_t>(sizeof(float))
    );

    shape->optimizeConvexHull();
    shape->initializePolyhedralFeatures();
    point_count = static_cast<uint32_t>(point_cloud.size() / 3);
}

void to_json(json& j, ConvexHullCollider const& p)
{
    to_json(j["local_transform"], p.local_transform);
}

void from_json(json const& j, ConvexHullCollider& p)
{
    from_json(j["local_transform"], p.local_transform);
}
}