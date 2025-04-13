#include "pose_animator.hpp"

#include <cmath>
#include <utility>
#include <animation/animation_data.hpp>
#include <animation/animation_clip.hpp>

namespace xen {
void PoseAnimator::update_animation(float const delta_time)
{
    if (current_animation_clip) {
        current_time += current_animation_clip->get_ticks_per_second() * delta_time;
        if (play_clip_indefinitely) {
            current_time = std::fmod(current_time, current_animation_clip->get_duration());
        }

        calculate_bone_transform(current_animation_clip->get_root_bone(), Matrix4(1.f));
    }
}

void PoseAnimator::set_animation_clip(std::shared_ptr<AnimationClip> clip)
{
    current_animation_clip = std::move(clip);
    current_time = 0.0f;
}

void PoseAnimator::calculate_bone_transform(AssimpBoneData const* node, Matrix4 const& parent_transform)
{
    std::string_view node_name = node->name;
    Matrix4 node_transform = node->transform;

    // Get the current bone engaged in the animation
    Bone* bone = current_animation_clip->find_bone(node_name);
    if (bone) {
        // Update the bones local transform with the current keyframe(s) data when we supply it the current animation
        // time (will blend between the keyframes)
        bone->update(current_time);
        node_transform = bone->get_local_transform();
    }

    // Calculate the total transformation given its parent
    Matrix4 const global_transform = parent_transform * node_transform;

    // We need to apply the inverse bind pose to our globalTransformation. This is necessary because the model starts in
    // bind pose and you need to animate a vertex, you need to transform it to the bone's local coordinate system,
    // calculate the transformation and move it back into world space in the shader
    auto& bone_data = current_animation_clip->get_bone_data();

    if (std::string node_name_str = std::string(node_name); bone_data.find(node_name_str) != bone_data.end()) {
        uint const index = bone_data[node_name_str].id;

        Log::rt_assert(
            index < max_bones_per_model, "Render::PoseAnimator "
                                         "We exceeded the MaxBonesPerModel limit"
        );

        Matrix4 const inverse_bind_pose = bone_data[node_name_str].inverse_bind_pos;
        final_bone_matrices[index] =
            current_animation_clip->get_global_inverse_transform() * global_transform * inverse_bind_pose;
    }

    for (auto const& child : node->children) {
        calculate_bone_transform(&child, global_transform);
    }
}
}