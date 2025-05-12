#pragma once

#include <component.hpp>

namespace xen {
class Ray;
struct RayHit;
class Shape;
enum class ShapeType;

class Collider final : public Component {
public:
    Collider() = default;
    explicit Collider(Shape&& shape);
    Collider(Collider const&) = delete;
    Collider(Collider&&) noexcept = default;
    Collider& operator=(Collider const&) = delete;
    Collider& operator=(Collider&&) noexcept = default;

    ShapeType get_shape_type() const { return shape_type; }

    bool has_shape() const { return (collider_shape != nullptr); }

    Shape const& get_shape() const
    {
        Log::rt_assert(has_shape(), "Error: No collider shape defined.");
        return *collider_shape;
    }

    Shape& get_shape()
    {
        Log::rt_assert(has_shape(), "Error: No collider shape defined.");
        return *collider_shape;
    }

    template <typename ShapeT>
    ShapeT const& get_shape() const;

    template <typename ShapeT>
    ShapeT& get_shape()
    {
        return const_cast<ShapeT&>(static_cast<Collider const*>(this)->get_shape<ShapeT>());
    }

    void set_shape(Shape&& shape);

    bool intersects(Collider const& collider) const { return intersects(*collider.collider_shape); }

    bool intersects(Shape const& shape) const;

    bool intersects(Ray const& ray, RayHit* hit = nullptr) const;

private:
    ShapeType shape_type{};
    std::unique_ptr<Shape> collider_shape{};
};
}

#include "collider.inl"