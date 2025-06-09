#pragma once

#include "collider.hpp"

class btCapsuleShape;

namespace xen {
class XEN_API CapsuleCollider : public Collider {

public:
    explicit CapsuleCollider(float radius = 0.5f, float height = 1.0f, Transform const& local_transform = {});
    ~CapsuleCollider();

    btCollisionShape* get_collision_shape() const override;

    float get_radius() const { return radius; }
    void set_radius(float radius);

    float get_height() const { return height; }
    void set_height(float height);

private:
    std::unique_ptr<btCapsuleShape> shape;
    float radius;
    float height;
};
}