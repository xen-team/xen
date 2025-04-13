#pragma once

namespace xen {
constexpr auto max_bones_per_model = 100; // Also will need to be adjusted in the appropriate skinned shaders
constexpr auto max_bones_per_vertex = 4;  // Also will need to be adjusted in the appropriate skinned shaders

// Data structure used for loading bone data from assimp and storing into our vertex buffers
struct VertexBoneData {
    std::array<uint32_t, max_bones_per_vertex> bone_ids;
    std::array<float, max_bones_per_vertex> weights;
};

struct BoneData {
    uint32_t id;

    /*
    Matrix that transforms from bone space to mesh space in bind pose.
    This matrix describes the position of the mesh in the local space of this bone when the skeleton was bound.
    Thus it can be used directly to determine a desired vertex position, given the world-space transform of the bone
    when animated, and the position of the vertex in mesh space.
    */
    Matrix4 inverse_bind_pos;

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(id, inverse_bind_pos);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(id, inverse_bind_pos);
    }
};
}