#pragma once
#include <animation/pose_animator.hpp>
#include <scene/component.hpp>

namespace xen {
struct PoseAnimatorComponent : public Component {
    inline static bool const registered = register_component<PoseAnimatorComponent>();

    PoseAnimator pose_animator;

    template <class Archive>
    void save(Archive& ar) const
    {
        // ar(pose_animator);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        // ar(pose_animator);
    }
};
}