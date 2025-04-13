#pragma once

#include "collider.hpp"

class btSphereShape;

namespace xen {
class XEN_API SphereCollider : public Collider::Registrar<SphereCollider> {
    inline static bool const registered = Register("sphere");

private:
    std::unique_ptr<btSphereShape> shape;
    float radius;

public:
    explicit SphereCollider(float radius = 0.5f, TransformComponent const& local_transform = {});

    btCollisionShape* get_collision_shape() const override;

    float get_radius() const { return radius; }
    void set_radius(float radius);

    friend void to_json(json& j, SphereCollider const& p);
    friend void from_json(json const& j, SphereCollider& p);
};
}