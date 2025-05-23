#pragma once

#include "collider.hpp"

class btConvexHullShape;

namespace xen {
class XEN_API ConvexHullCollider : public Collider {
public:
    explicit ConvexHullCollider(std::vector<float> const& point_cloud = {}, Transform const& local_transform = {});

    btCollisionShape* get_collision_shape() const override;

    uint32_t get_point_count() const { return point_count; }
    void set_point_count(std::vector<float> const& point_cloud);

private:
    std::unique_ptr<btConvexHullShape> shape;
    uint32_t point_count = 0;
};
}