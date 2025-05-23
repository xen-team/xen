#include "skeleton.hpp"

/*

namespace xen {
template <>
void SkeletonJoint::rotate(Quaternion const& rotation)
{
    this->rotation *= rotation;

    for (SkeletonJoint* child : children) {
        child->rotate(rotation);
    }
}

template <>
Matrix4 SkeletonJoint::compute_transform() const
{
    Matrix4 transform_mat = rotation.compute();
    transform_mat[3][0] = translation.x;
    transform_mat[3][1] = translation.y;
    transform_mat[3][2] = translation.z;

    return transform_mat;
}
}
*/