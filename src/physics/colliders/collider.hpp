#pragma once

#include <scene/components/transform.hpp>
#include <utils/json_factory.hpp>
#include <rocket.hpp>

class btCollisionShape;
class btVector3;
class btQuaternion;
class btTransform;

namespace xen {
class XEN_API Collider : public JsonFactory<Collider> {
protected:
    TransformComponent local_transform;

public:
    rocket::signal<void(Collider*, TransformComponent const&)> on_transform_change;

public:
    explicit Collider(TransformComponent const& local_transform = {});

    virtual btCollisionShape* get_collision_shape() const = 0;

    TransformComponent const& get_local_transform() const { return local_transform; }
    void set_local_transform(TransformComponent const& local_transform);

    static btVector3 convert(Vector3f const& vector);
    static Vector3f convert(btVector3 const& vector);
    static btQuaternion convert(Quaternion const& quaternion);
    static Quaternion convert(btQuaternion const& quaternion);
    static btTransform convert(TransformComponent const& transform);
    static TransformComponent convert(btTransform const& transform, Vector3f const& scaling = Vector3f(1.0f));
};
}