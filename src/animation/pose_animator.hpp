#pragma once

namespace xen {
class AnimationClip;
struct AssimpBoneData;

class PoseAnimator {
private:
    std::vector<Matrix4> final_bone_matrices = std::vector<Matrix4>(100, Matrix4(1.f));
    std::shared_ptr<AnimationClip> current_animation_clip;
    float current_time = 0.f;

    bool play_clip_indefinitely = true;

public:
    void update_animation(float delta_time);
    void set_animation_clip(std::shared_ptr<AnimationClip> clip);

    [[nodiscard]] AnimationClip const* get_current_animation_clip() { return current_animation_clip.get(); }
    [[nodiscard]] std::vector<Matrix4> const& GetFinalBoneMatrices() const { return final_bone_matrices; }

    template <class Archive>
    void save(Archive& ar) const
    {
        // ar(final_bone_matrices, current_animation_clip, current_time, play_clip_indefinitely);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        // ar(final_bone_matrices, current_animation_clip, current_time, play_clip_indefinitely);
    }

private:
    void calculate_bone_transform(AssimpBoneData const* node, Matrix4 const& parent_transform);
};
}