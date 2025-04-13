#pragma once

#include "bone.hpp"

#include <assimp/anim.h>

struct aiNode;

namespace xen {
class Model;
class BoneData;

struct AssimpBoneData {
    Matrix4 transform;
    std::string name;
    uint32_t child_count;
    std::vector<AssimpBoneData> children;

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(child_count);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(transform, name, child_count, children);
    }
};

class AnimationClip {
private:
    float clip_duration;
    float ticks_per_second;
    std::vector<Bone> bones;
    AssimpBoneData root_node;

    // #if !ARC_FINAL
    //     std::string m_AnimationName;
    // #endif
    Model* model;

public:
    AnimationClip(std::string_view animation_path, uint animation_index, Model& model);

    Bone* find_bone(std::string_view name);

    [[nodiscard]] float get_duration() const { return clip_duration; }

    [[nodiscard]] float get_ticks_per_second() const { return ticks_per_second; }

    [[nodiscard]] AssimpBoneData const* get_root_bone() const { return &root_node; }

    [[nodiscard]] std::unordered_map<std::string, BoneData>& get_bone_data() const;

    [[nodiscard]] Matrix4 const& get_global_inverse_transform() const;

    // #if !ARC_FINAL
    //     std::string GetAnimationName() { return m_AnimationName; }
    // #endif

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(clip_duration, ticks_per_second, bones, root_node, model);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(clip_duration, ticks_per_second, bones, root_node, model);
    }

private:
    void read_missing_bones(aiAnimation const* assimp_animation);

    void read_hierarchy_data(AssimpBoneData& dest, aiNode const* src);
};
}