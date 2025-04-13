#pragma once

#include "collider.hpp"

class btConvexHullShape;

namespace xen {
class XEN_API ConvexHullCollider : public Collider::Registrar<ConvexHullCollider> {
    inline static bool const registered = Register("convex_hull");

private:
    std::unique_ptr<btConvexHullShape> shape;
    uint32_t point_count = 0;

public:
    explicit ConvexHullCollider(
        std::vector<float> const& point_cloud = {}, TransformComponent const& local_transform = {}
    );

    btCollisionShape* get_collision_shape() const override;

    uint32_t get_point_count() const { return point_count; }
    void set_point_count(std::vector<float> const& point_cloud);

    friend void to_json(json& j, ConvexHullCollider const& p);
    friend void from_json(json const& j, ConvexHullCollider& p);
};
}