#pragma once

#include "collider.hpp"

class btSphereShape;

namespace xen {
class XEN_API SphereCollider : public Collider {
public:
    explicit SphereCollider(float radius = 0.5f, Transform const& local_transform = {});

    btCollisionShape* get_collision_shape() const override;

    float get_radius() const { return radius; }
    void set_radius(float radius);

private:
    std::unique_ptr<btSphereShape> shape;
    float radius;
};
}