#pragma once

#include "collider.hpp"

class btCapsuleShape;

namespace xen {
class XEN_API CapsuleCollider : public Collider::Registrar<CapsuleCollider> {
    inline static bool const registered = Register("capsule");

private:
    std::unique_ptr<btCapsuleShape> shape;
    float radius;
    float height;

public:
    explicit CapsuleCollider(float radius = 0.5f, float height = 1.0f, TransformComponent const& local_transform = {});
    ~CapsuleCollider() override;

    btCollisionShape* get_collision_shape() const override;

    float get_radius() const { return radius; }
    void set_radius(float radius);

    float get_height() const { return height; }
    void set_height(float height);

    friend void to_json(json& j, CapsuleCollider const& p);
    friend void from_json(json const& j, CapsuleCollider& p);
};
}