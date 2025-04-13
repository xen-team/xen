#pragma once

#include "collider.hpp"

class btConeShape;

namespace xen {
class XEN_API ConeCollider : public Collider::Registrar<ConeCollider> {
    inline static bool const registered = Register("cone");

private:
    std::unique_ptr<btConeShape> shape;
    float radius;
    float height;

public:
    explicit ConeCollider(float radius = 1.0f, float height = 1.0f, TransformComponent const& local_transform = {});

    btCollisionShape* get_collision_shape() const override;

    float get_radius() const { return radius; }
    void set_radius(float radius);

    float get_height() const { return height; }
    void set_height(float height);

    friend void to_json(json& j, ConeCollider const& p);
    friend void from_json(json const& j, ConeCollider& p);
};
}