#pragma once

class btCollisionShape;
class btVector3;
class btQuaternion;
class btTransform;

namespace xen {
class XEN_API Collider {
public:
    explicit Collider(Transform const& local_transform = {});

    virtual btCollisionShape* get_collision_shape() const = 0;

    Transform const& get_local_transform() const { return local_transform; }
    void set_local_transform(Transform const& local_transform);

    static btVector3 convert(Vector3f const& vector);
    static Vector3f convert(btVector3 const& vector);
    static btQuaternion convert(Quaternion const& quaternion);
    static Quaternion convert(btQuaternion const& quaternion);
    static btTransform convert(Transform const& transform);
    static Transform convert(btTransform const& transform, Vector3f const& scaling = Vector3f(1.0f));

protected:
    Transform local_transform;
};
}