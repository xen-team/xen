#include "animation_clip.hpp"
#include "animation_data.hpp"

#include <render/mesh/model.hpp>
#include <assimp/Importer.hpp>
#include <utility>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace xen {
AnimationClip::AnimationClip(std::string_view animation_path, uint const animation_index, Model& model) : model(&model)
{
    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFile(animation_path.data(), aiProcess_Triangulate);

    Log::rt_assert(
        scene && scene->mRootNode, "Render::AnimationClip "
                                   "Failed importing animationPath"
    );

    auto* const assimp_animation = scene->mAnimations[animation_index];
    // #if !ARC_FINAL
    //     if (assimpAnimation->mName.length > 0)
    //         m_AnimationName = std::string(assimpAnimation->mName.C_Str());
    // #endif
    clip_duration = static_cast<float>(assimp_animation->mDuration);
    ticks_per_second =
        assimp_animation->mTicksPerSecond != 0 ? static_cast<float>(assimp_animation->mTicksPerSecond) : 1.0f;

    // Setup our root node bone and recursively build the others
    read_hierarchy_data(root_node, scene->mRootNode);
    read_missing_bones(assimp_animation);
}

Bone* AnimationClip::find_bone(std::string_view name)
{
    auto iter = std::ranges::find_if(bones, [&](Bone const& bone) { return bone.get_name() == name; });
    return iter == bones.end() ? nullptr : &(*iter);
}

void AnimationClip::read_missing_bones(aiAnimation const* assimp_animation)
{
    auto const size = assimp_animation->mNumChannels;

    auto& bone_info_map = model->get_bone_data();
    auto& bone_count = model->get_bone_count();

    // Sometimes we miss bones, so this function will find any other bones engaged in the animation and add them. ASSimp
    // struggles..
    for (uint i = 0; i < size; i++) {
        auto* const channel = assimp_animation->mChannels[i];
        std::string const bone_name = channel->mNodeName.data;

        if (!bone_info_map.contains(bone_name)) {
            bone_info_map[bone_name].id = bone_count++;
        }

        bones.emplace_back(bone_name, bone_info_map[bone_name].id, channel);
    }
}

void AnimationClip::read_hierarchy_data(AssimpBoneData& dest, aiNode const* src)
{
    Log::rt_assert(
        src, "Render::AnimationClip "
             "Needs src data to read in AnimationClip"
    );

    uint const children_count = src->mNumChildren;

    dest.name = src->mName.data;
    dest.transform = Model::convert_assimp_matrix(src->mTransformation);
    dest.child_count = children_count;
    dest.children.reserve(dest.child_count);

    for (uint i = 0; i < children_count; i++) {
        AssimpBoneData new_data;
        read_hierarchy_data(new_data, src->mChildren[i]);
        dest.children.emplace_back(std::move(new_data));
    }
}

std::unordered_map<std::string, BoneData>& AnimationClip::get_bone_data() const
{
    return model->get_bone_data();
}

Matrix4 const& AnimationClip::get_global_inverse_transform() const
{
    return model->get_global_inverse_transform();
}
}