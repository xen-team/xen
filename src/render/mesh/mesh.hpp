#pragma once
#include "material.hpp"

#include <animation/animation_data.hpp>
#include <utility>

namespace xen {
class Mesh {
    friend class Model;
    friend class AssetManager;

    // This works great for loading in different types of data into our vertex buffers. This will no longer be a valid
    // strategy if we ever add a data type that isn't the same size When that happens we should rework how we are
    // loading in data anyways, since it will be a nice memory and speed optimization anyways. For now, this will do!
    union BufferData {
        float floating_point;
        int integer;
        BufferData(int i) : integer(i) {}
        BufferData(float f) : floating_point(f) {}

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(floating_point, integer);
        }
    };

protected:
    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ibo = 0;
    Material material;

    std::vector<Vector3f> positions;
    std::vector<Vector2f> uvs;
    std::vector<Vector3f> normals;
    std::vector<Vector3f> tangents;
    std::vector<Vector3f> bitangents;
    std::vector<VertexBoneData> bone_data;

    std::vector<uint> indices;

    std::vector<BufferData> buffer_data;
    bool is_interleaved;
    uint32_t buffer_component_count;

public:
    Mesh(
        std::vector<Vector3f> positions = {}, std::vector<Vector2f> uvs = {}, std::vector<Vector3f> normals = {},
        std::vector<Vector3f> tangents = {}, std::vector<Vector3f> bitangents = {},
        std::vector<VertexBoneData> bone_weights = {}, std::vector<uint32_t> indices = {}
    ) :
        positions{std::move(positions)}, uvs{std::move(uvs)}, normals{std::move(normals)},
        tangents{std::move(tangents)}, bitangents{std::move(bitangents)}, bone_data{std::move(bone_weights)},
        indices{std::move(indices)} {};

    void load_data(bool interleaved = true);

    void generate_gpu_data(); // Commits all of the buffers and their attributes to the GPU driver

    void draw() const;

    [[nodiscard]] Material& get_material() { return material; }

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(vao, vbo, ibo, positions, uvs, normals, tangents, bitangents, bone_data, indices, buffer_data,
           is_interleaved, buffer_component_count);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(vao, vbo, ibo, positions, uvs, normals, tangents, bitangents, bone_data, indices, buffer_data,
           is_interleaved, buffer_component_count);
    }

private:
    void setup_interleaved_attributes();
    
    void setup_separate_attributes();
};
}