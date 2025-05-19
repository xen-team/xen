#include "collider.hpp"

#include <utils/shape.hpp>

namespace xen {
Collider::Collider(Shape&& shape)
{
    set_shape(std::move(shape));
}

void Collider::set_shape(Shape&& shape)
{
    shape_type = shape.get_type();

    switch (shape_type) {
    case ShapeType::LINE:
        collider_shape = std::make_unique<Line>(static_cast<Line&&>(shape));
        break;

    case ShapeType::PLANE:
        collider_shape = std::make_unique<Plane>(static_cast<Plane&&>(shape));
        break;

    case ShapeType::SPHERE:
        collider_shape = std::make_unique<Sphere>(static_cast<Sphere&&>(shape));
        break;

    case ShapeType::TRIANGLE:
        collider_shape = std::make_unique<Triangle>(static_cast<Triangle&&>(shape));
        break;

    case ShapeType::QUAD:
        collider_shape = std::make_unique<Quad>(static_cast<Quad&&>(shape));
        break;

    case ShapeType::AABB:
        collider_shape = std::make_unique<AABB>(static_cast<AABB&&>(shape));
        break;

    case ShapeType::OBB:
        collider_shape = std::make_unique<OBB>(static_cast<OBB&&>(shape));
        break;

    default:
        throw std::invalid_argument("Error: Unhandled shape type in the collider shape setter");
    }
}

bool Collider::intersects(Shape const& shape) const
{
    switch (shape_type) {
    case ShapeType::LINE:
        return shape.intersects(static_cast<Line const&>(*collider_shape));

    case ShapeType::PLANE:
        return shape.intersects(static_cast<Plane const&>(*collider_shape));

    case ShapeType::SPHERE:
        return shape.intersects(static_cast<Sphere const&>(*collider_shape));

    case ShapeType::TRIANGLE:
        return shape.intersects(static_cast<Triangle const&>(*collider_shape));

    case ShapeType::QUAD:
        return shape.intersects(static_cast<Quad const&>(*collider_shape));

    case ShapeType::AABB:
        return shape.intersects(static_cast<AABB const&>(*collider_shape));

    case ShapeType::OBB:
        return shape.intersects(static_cast<const OBB&>(*collider_shape));

    default:
        break;
    }

    throw std::invalid_argument("Error: Unhandled shape type in the collider/shape intersection check");
}

bool Collider::intersects(Ray const& ray, RayHit* hit) const
{
    switch (shape_type) {
    case ShapeType::LINE:
        // return ray.intersects(static_cast<const Line&>(*collider_shape), hit);
        break;

    case ShapeType::PLANE:
        return ray.intersects(dynamic_cast<Plane const&>(*collider_shape), hit);

    case ShapeType::SPHERE:
        return ray.intersects(dynamic_cast<Sphere const&>(*collider_shape), hit);

    case ShapeType::TRIANGLE:
        return ray.intersects(dynamic_cast<Triangle const&>(*collider_shape), hit);

    case ShapeType::QUAD:
        // return ray.intersects(static_cast<const Quad&>(*collider_shape), hit);
        break;

    case ShapeType::AABB:
        return ray.intersects(dynamic_cast<AABB const&>(*collider_shape), hit);

    case ShapeType::OBB:
        // return ray.intersects(static_cast<const OBB&>(*collider_shape), hit);
        break;

    default:
        break;
    }

    throw std::invalid_argument("Error: Unhandled shape type in the collider/ray intersection check");
}
}