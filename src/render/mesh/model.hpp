#pragma once

#include "mesh.hpp"

#ifndef RENDERPASSTYPE_H
#include <render/renderer/renderpass/render_pass_type.hpp>
#endif

#include <assimp/material.h>
#include <assimp/matrix4x4.h>

struct aiScene;
struct aiMaterial;
struct aiNode;
struct aiMesh;

namespace xen {
class Shader;

class Model {
    friend class Resources;

private:
    std::vector<Mesh> meshes;
    std::unordered_map<std::string, BoneData> bone_data;
    Matrix4 global_inverse_transform; // Used by animation for bone related data to move it back to the origin
    uint32_t bone_count = 0;

    std::string directory;
    std::string name;

public:
    Model() = default;
    Model(Mesh const& mesh);
    Model(std::vector<Mesh> const& meshes);

    void draw(Shader& shader, RenderPassType pass);

    [[nodiscard]] std::span<Mesh> get_meshes() { return meshes; }

    [[nodiscard]] std::string_view get_name() const { return name; }

    [[nodiscard]] auto& get_bone_data() { return bone_data; }
    [[nodiscard]] uint32_t& get_bone_count() { return bone_count; }
    [[nodiscard]] auto const& get_global_inverse_transform() { return global_inverse_transform; }

    [[nodiscard]] static Matrix4 convert_assimp_matrix(aiMatrix4x4 const& ai_mat)
    {
        return Matrix4({ai_mat.a1, ai_mat.a2, ai_mat.a3, ai_mat.a4, ai_mat.b1, ai_mat.b2, ai_mat.b3, ai_mat.b4,
                        ai_mat.c1, ai_mat.c2, ai_mat.c3, ai_mat.c4, ai_mat.d1, ai_mat.d2, ai_mat.d3, ai_mat.d4})
            .transpose();
    }

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(meshes, bone_data, global_inverse_transform, bone_count, directory, name);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(meshes, bone_data, global_inverse_transform, bone_count, directory, name);
    }

private:
    void load_model(std::string_view path);
    void generate_gpu_data();

    void process_indices(aiMesh* mesh, std::vector<uint32_t>& indices);
    void process_bones(
        aiMesh* mesh, std::unordered_map<std::string, BoneData>& bone_data, std::vector<VertexBoneData>& bone_weights
    );
    void process_materials(aiMesh* mesh, aiScene const* scene, Mesh& new_mesh);

    void process_node(aiNode* node, aiScene const* scene);
    void process_mesh(aiMesh* mesh, aiScene const* scene);
    Texture* load_material_texture(aiMaterial* mat, aiTextureType type, bool is_srgb);
};
}