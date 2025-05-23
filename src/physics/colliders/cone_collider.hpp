#pragma once

#include "collider.hpp"

class btConeShape;

namespace xen {
class XEN_API ConeCollider : public Collider {
public:
    explicit ConeCollider(float radius = 1.0f, float height = 1.0f, Transform const& local_transform = {});

    btCollisionShape* get_collision_shape() const override;

    float get_radius() const { return radius; }
    void set_radius(float radius);

    float get_height() const { return height; }
    void set_height(float height);

private:
    std::unique_ptr<btConeShape> shape;
    float radius;
    float height;
};
}