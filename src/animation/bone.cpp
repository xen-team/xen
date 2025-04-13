#include "bone.hpp"
#include <assimp/anim.h>

namespace xen {
Bone::Bone(std::string_view const name, uint32_t const id, aiNodeAnim const* channel) : name(name), id(id)
{
    positions.reserve(channel->mNumPositionKeys);
    rotations.reserve(channel->mNumRotationKeys);
    scales.reserve(channel->mNumScalingKeys);

    for (uint i = 0; i < channel->mNumPositionKeys; i++) {
        aiVector3D const& ai_position = channel->mPositionKeys[i].mValue;

        KeyPosition data;
        data.position = Vector3f(ai_position.x, ai_position.y, ai_position.z);
        data.timestamp = static_cast<float>(channel->mPositionKeys[i].mTime);
        positions.emplace_back(std::move(data));
    }
    for (uint i = 0; i < channel->mNumRotationKeys; i++) {
        aiQuaternion const& ai_orientation = channel->mRotationKeys[i].mValue;

        KeyRotation data;
        data.orientation = Quaternion(ai_orientation.w, ai_orientation.x, ai_orientation.y, ai_orientation.z);
        data.timestamp = static_cast<float>(channel->mRotationKeys[i].mTime);
        rotations.emplace_back(std::move(data));
    }
    for (uint i = 0; i < channel->mNumScalingKeys; i++) {
        aiVector3D const& scale = channel->mScalingKeys[i].mValue;

        KeyScale data;
        data.scale = Vector3f(scale.x, scale.y, scale.z);
        data.timestamp = static_cast<float>(channel->mScalingKeys[i].mTime);
        scales.emplace_back(std::move(data));
    }
}

// Interpolates between positions, rotations, and scaling keys based on the current timestep were at in the animation
// and prepares the local transform for the bone given this info
void Bone::update(float const current_animation_time)
{
    Matrix4 const translation = interpolate_position(current_animation_time);
    Matrix4 const rotation = interpolate_rotation(current_animation_time);
    Matrix4 const scale = interpolate_scale(current_animation_time);
    local_transform = translation * rotation * scale;
}

float Bone::get_normalized_interpolation_between_frames(
    float const last_timestamp, float const next_timestamp, float const current_animation_time
) const
{
    float const widway_length = current_animation_time - last_timestamp;
    float const frames_diff = next_timestamp - last_timestamp;
    return widway_length / frames_diff;
}

Matrix4 Bone::interpolate_position(float const current_animation_time) const
{
    if (positions.size() == 1) {
        return Matrix4(1.f).translate(positions[0].position);
    }

    uint32_t const index0 = get_position_index(current_animation_time);
    uint32_t const index1 = index0 + 1;
    float const lerp_value = get_normalized_interpolation_between_frames(
        positions[index0].timestamp, positions[index1].timestamp, current_animation_time
    );

    // Finally LERP between our position data for our animation frames
    Vector3f const final_position = positions[index0].position.lerp(positions[index1].position, lerp_value);
    return Matrix4(1.f).translate(final_position);
}

Matrix4 Bone::interpolate_rotation(float const current_animation_time) const
{
    if (rotations.size() == 1) {
        auto const rotation = rotations[0].orientation.normalize();
        return rotation.to_matrix();
    }

    uint32_t const index0 = get_rotation_index(current_animation_time);
    uint32_t const index1 = index0 + 1;
    float const slerp_value = get_normalized_interpolation_between_frames(
        rotations[index0].timestamp, rotations[index1].timestamp, current_animation_time
    );

    // Finally SLERP between our rotation data for our animation frames
    Quaternion const final_rotation =
        rotations[index0].orientation.slerp(rotations[index1].orientation, slerp_value).normalize();

    return final_rotation.to_matrix();
}

Matrix4 Bone::interpolate_scale(float const current_animation_time) const
{
    if (scales.size() == 1) {
        return Matrix4(1.f).scale(scales[0].scale);
    }

    uint32_t const index0 = get_scale_index(current_animation_time);
    uint32_t const index1 = index0 + 1;
    float const lerp_value = get_normalized_interpolation_between_frames(
        scales[index0].timestamp, scales[index1].timestamp, current_animation_time
    );

    // Finally LERP between our scale data for our animation frames
    Vector3f const final_scale = scales[index0].scale.lerp(scales[index1].scale, lerp_value);

    return Matrix4(1.f).scale(final_scale);
}

// Do these vectors sorted for binary search? cuz idk
uint32_t Bone::get_position_index(float const current_animation_time) const
{
    auto comparator = [](KeyPosition const& x, KeyPosition const& y) { return x.timestamp < y.timestamp; };
    auto const result =
        std::ranges::upper_bound(positions, KeyPosition{.timestamp = current_animation_time}, comparator);

    if (result != positions.end()) {
        return static_cast<uint32_t>(std::distance(positions.begin(), result));
    }

    Log::rt_assert(
        false, "Render::Bone "
               "No position keyframe was found, something is wrong"
    );
    return 0;
}

uint32_t Bone::get_rotation_index(float const current_animation_time) const
{
    auto comparator = [](KeyRotation const& x, KeyRotation const& y) { return x.timestamp < y.timestamp; };
    auto const result =
        std::ranges::upper_bound(rotations, KeyRotation{.timestamp = current_animation_time}, comparator);

    if (result != rotations.end()) {
        return static_cast<uint32_t>(std::distance(rotations.begin(), result));
    }

    Log::rt_assert(
        false, "Render::Bone "
               "No rotation keyframe was found, something is wrong"
    );
    return 0;
}

uint32_t Bone::get_scale_index(float const current_animation_time) const
{
    auto comparator = [](KeyScale const& x, KeyScale const& y) { return x.timestamp < y.timestamp; };
    auto const result = std::ranges::upper_bound(scales, KeyScale{.timestamp = current_animation_time}, comparator);

    if (result != scales.end()) {
        return static_cast<uint32_t>(std::distance(scales.begin(), result));
    }

    Log::rt_assert(
        false, "Render::Bone "
               "No scale keyframe was found, something is wrong"
    );
    return 0;
}

}