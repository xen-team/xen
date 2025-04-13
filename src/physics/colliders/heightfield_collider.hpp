#pragma once

#include "collider.hpp"

class btHeightfieldTerrainShape;

namespace xen {
class XEN_API HeightfieldCollider : public Collider::Registrar<HeightfieldCollider> {
    inline static bool const registered = Register("heightfield");

private:
    std::unique_ptr<btHeightfieldTerrainShape> shape;

public:
    explicit HeightfieldCollider(
        int32_t height_stick_width = 100, int32_t height_stick_length = 100, void const* heightfield_data = nullptr,
        float min_height = -1.0f, float max_height = 1.0f, bool flip_quad_edges = false,
        TransformComponent const& local_transform = {}
    );

    btCollisionShape* get_collision_shape() const override;

    void set_heightfield(
        int32_t height_stick_width, int32_t height_stick_length, void const* heightfield_data, float min_height,
        float max_height, bool flip_quad_edges
    );

    friend void to_json(json& j, HeightfieldCollider const& p);
    friend void from_json(json const& j, HeightfieldCollider& p);
};
}