#include "heightfield_collider.hpp"
#include "math/math_serialization.hpp"

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

namespace xen {
HeightfieldCollider::HeightfieldCollider(
    int32_t const height_stick_width, int32_t const height_stick_length, void const* heightfield_data,
    float const min_height, float const max_height, bool const flip_quad_edges,
    TransformComponent const& local_transform
) /*:
Collider(local_transform)*/
{
    this->local_transform = local_transform;
    set_heightfield(height_stick_width, height_stick_length, heightfield_data, min_height, max_height, flip_quad_edges);
}

btCollisionShape* HeightfieldCollider::get_collision_shape() const
{
    return shape.get();
}

void HeightfieldCollider::set_heightfield(
    int32_t const height_stick_width, int32_t const height_stick_length, void const* heightfield_data,
    float const min_height, float const max_height, bool const flip_quad_edges
)
{
    if (!heightfield_data) {
        return;
    }

    shape = std::make_unique<btHeightfieldTerrainShape>(
        height_stick_width, height_stick_length, heightfield_data, 1.0f, min_height, max_height, 1, PHY_FLOAT,
        flip_quad_edges
    );
}

void to_json(json& j, HeightfieldCollider const& p)
{
    to_json(j["local_transform"], p.local_transform);
}

void from_json(json const& j, HeightfieldCollider& p)
{
    from_json(j["local_transform"], p.local_transform);
}
}