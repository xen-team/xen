#pragma once

#include <data/graph.hpp>

namespace xen {
class SkeletonJoint final : public GraphNode<SkeletonJoint> {
public:
    SkeletonJoint() = default;
    SkeletonJoint(Quaternion const& rotation, Vector3f const& translation) :
        rotation{rotation}, translation{translation}
    {
    }

    SkeletonJoint(SkeletonJoint const&) = delete;
    SkeletonJoint(SkeletonJoint&&) noexcept = default;

    SkeletonJoint& operator=(SkeletonJoint const&) = delete;
    SkeletonJoint& operator=(SkeletonJoint&& joint) = default;

    constexpr Quaternion const& get_rotation() const { return rotation; }

    constexpr Vector3f const& get_translation() const { return translation; }

    void set_rotation(Quaternion const& rotation) { this->rotation = rotation; }

    void set_translation(Vector3f const& translation) { this->translation = translation; }

    // void rotate(Quaternion const& rotation);

    void translate(Vector3f const& translation) { this->translation += translation; }

    // Matrix4 compute_transform() const;

private:
    Quaternion rotation = Quaternion::Identity;
    Vector3f translation{};
};

using Skeleton = Graph<SkeletonJoint>;
}
