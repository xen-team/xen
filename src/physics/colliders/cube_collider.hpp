#pragma once

#include "collider.hpp"

class btBoxShape;

namespace xen {
class XEN_API CubeCollider : public Collider::Registrar<CubeCollider> {
    inline static bool const registered = Register("cube");

private:
    std::unique_ptr<btBoxShape> shape;
    Vector3f extents;

public:
    explicit CubeCollider(Vector3f const& extents = Vector3f(1.0f), TransformComponent const& local_transform = {});

    btCollisionShape* get_collision_shape() const override;

    Vector3f const& get_extents() const { return extents; }
    void set_extents(Vector3f const& extents);

    friend void to_json(json& j, CubeCollider const& p);
    friend void from_json(json const& j, CubeCollider& p);
};
}