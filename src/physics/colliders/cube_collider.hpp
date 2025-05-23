#pragma once

#include "collider.hpp"

class btBoxShape;

namespace xen {
class XEN_API CubeCollider : public Collider {
public:
    explicit CubeCollider(Vector3f const& extents = Vector3f(1.0f), Transform const& local_transform = {});

    btCollisionShape* get_collision_shape() const override;

    Vector3f const& get_extents() const { return extents; }
    void set_extents(Vector3f const& extents);

private:
    std::unique_ptr<btBoxShape> shape;
    Vector3f extents;
};
}