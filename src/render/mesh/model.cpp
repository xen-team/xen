#include "model.hpp"

#include <render/shader.hpp>
#include <animation/animation_data.hpp>
#include <render/texture/texture.hpp>
#include <resource/resources.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace xen {
Model::Model(Mesh const& mesh)
{
    meshes.emplace_back(mesh);
}

Model::Model(std::vector<Mesh> const& meshes) : meshes(meshes) {}

void Model::draw(Shader& shader, RenderPassType const pass)
{
    bool const is_pass_meterial_required = pass == RenderPassType::MaterialRequired;

    for (auto& mesh : meshes) {
        // Avoid binding material information when it isn't needed
        if (is_pass_meterial_required) {
            mesh.material.bind_material_information(shader);
        }
        mesh.draw();
    }
}

void Model::load_model(std::string_view path)
{
    Assimp::Importer import;
    aiScene const* scene =
        import.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Log::verror("Render::Model failed to load model - {}", import.GetErrorString());
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    name = path.substr(path.find_last_of("/\\") + 1);

    process_node(scene->mRootNode, scene);
}

void Model::generate_gpu_data()
{
    for (auto& mesh : meshes) {
        mesh.generate_gpu_data();
    }
}

void Model::process_node(aiNode* node, aiScene const* scene)
{
    // Process all of the node's meshes (if any)
    for (uint i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh, scene);
    }

    // Process all of the node's children
    for (uint i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
}

void Model::process_mesh(aiMesh* mesh, aiScene const* scene)
{
    std::vector<Vector3f> positions;
    std::vector<Vector2f> uvs;
    std::vector<Vector3f> normals;
    std::vector<Vector3f> tangents;
    std::vector<Vector3f> bitangents;
    std::vector<uint32_t> indices;
    std::vector<VertexBoneData> bone_weights;

    positions.reserve(mesh->mNumVertices);
    uvs.reserve(mesh->mNumVertices);
    normals.reserve(mesh->mNumVertices);
    tangents.reserve(mesh->mNumVertices);
    bitangents.reserve(mesh->mNumVertices);
    if (mesh->mNumBones > 0) {
        bone_weights.reserve(mesh->mNumVertices);
    }
    indices.reserve(mesh->mNumFaces * 3);

    bool const has_texture_coords = mesh->mTextureCoords[0] != nullptr;

    // Process vertices
    for (uint i = 0; i < mesh->mNumVertices; ++i) {
        Vector2f const uv_coord = has_texture_coords ?
                                      Vector2f(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) :
                                      Vector2f(0.0f, 0.0f);

        positions.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        uvs.emplace_back(uv_coord);
        normals.emplace_back(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        tangents.emplace_back(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        bitangents.emplace_back(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

        if (mesh->mNumBones > 0) {
            std::ranges::fill(bone_weights[i].bone_ids, -1);
            std::ranges::fill(bone_weights[i].weights, 0.0f);
        }
    }

    // Save some animation related info
    global_inverse_transform = convert_assimp_matrix(scene->mRootNode->mTransformation).inverse();

    process_bones(mesh, bone_data, bone_weights);

    process_indices(mesh, indices);

    Mesh new_mesh(
        std::move(positions), std::move(uvs), std::move(normals), std::move(tangents), std::move(bitangents),
        std::move(bone_weights), std::move(indices)
    );
    new_mesh.load_data();

    process_materials(mesh, scene, new_mesh);

    meshes.emplace_back(std::move(new_mesh));
}

void Model::process_indices(aiMesh* mesh, std::vector<uint32_t>& indices)
{
    // Loop through every face (triangle thanks to aiProcess_Triangulate) and stores its indices in our meshes
    // indices.
    // This will ensure they are in the right order.
    for (uint i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (uint j = 0; j < face.mNumIndices; ++j) {
            indices.emplace_back(face.mIndices[j]);
        }
    }
}

void Model::process_bones(
    aiMesh* mesh, std::unordered_map<std::string, BoneData>& bone_data, std::vector<VertexBoneData>& bone_weights
)
{
    for (uint bone_index = 0; bone_index < mesh->mNumBones; bone_index++) {
        aiBone* bone = mesh->mBones[bone_index];

        // Get the bone name and if it doesn't exist let's add it to the bone array along with its matrix
        std::string const bone_name = bone->mName.C_Str();

        auto& bone_info = bone_data.emplace(bone_name, BoneData{}).first->second;
        if (bone_info.id == -1) {
            bone_info.id = bone_data.size() - 1;
            bone_info.inverse_bind_pos = convert_assimp_matrix(bone->mOffsetMatrix);
        }

        Log::rt_assert(
            bone_info.id != -1, "Render::Model "
                                "Bone not found or created.."
        );

        // Now let's go through every vertex this bone affects and attempt to add the weight and index of the bone to
        // that vertex
        aiVertexWeight* weights = bone->mWeights;
        for (uint weight_index = 0; weight_index < bone->mNumWeights; weight_index++) {
            int const vertex_id = weights[weight_index].mVertexId;
            float const current_weight = weights[weight_index].mWeight;

            Log::rt_assert(
                vertex_id < (int)mesh->mNumVertices, "Render::Model "
                                                     "Bone data is trying to access an vertex that doesn't exist"
            );

            // Let's attempt to add our bone weight and bone ID to the vertex data. It might be full since we limit how
            // many bones can influence a single vertex
            bool found_slot = false;
            for (int i = 0; i < max_bones_per_vertex; i++) {
                // Check if a slot is empty, if so mark it as found, and fill the slot
                if (bone_weights[vertex_id].bone_ids[i] == -1) {
                    bone_weights[vertex_id].bone_ids[i] = bone_info.id;
                    bone_weights[vertex_id].weights[i] = current_weight;
                    found_slot = true;
                    break;
                }
            }
            if (!found_slot) {
                // Since we haven't found an open slot left, let's iterate over our slots and keep track of the lowest
                // weight. This can be useful since all slots are full since we can replace a bone weight for the vertex
                // if another bone exists that has more influence. This is just working around bone vertex limitations,
                // in such a way that hopefully reduces the quality loss when doing skeletal animation
                float lowest_weight = 1.0f; // Maximum weight a bone can have on a vert
                int smallest_weight_index = -1;
                for (int i = 0; i < max_bones_per_vertex; i++) {
                    if (bone_weights[vertex_id].weights[i] < lowest_weight) {
                        lowest_weight = bone_weights[vertex_id].weights[i];
                        smallest_weight_index = i;
                    }
                }

                // Now let's check if we should replace
                if (current_weight > lowest_weight && smallest_weight_index != -1) {
                    Log::vwarning(
                        "Render::Model "
                        "Hit Bone Vertex Capacity {} on Vertex id:{} - Replacing bone:{} on the vert because it's "
                        "influence:{} is less than the bone:{} we're trying to add's influence:{}",
                        max_bones_per_vertex, vertex_id, bone_weights[vertex_id].bone_ids[smallest_weight_index],
                        bone_weights[vertex_id].weights[smallest_weight_index], bone_info.id, current_weight
                    );

                    bone_weights[vertex_id].bone_ids[smallest_weight_index] = bone_info.id;
                    bone_weights[vertex_id].weights[smallest_weight_index] = current_weight;
                }
                else {
                    Log::vwarning(
                        "Render::Model "
                        "Hit Bone Vertex Capacity {} on Vertex id:{} - Not adding bone:{}'s influence amount:{} "
                        "because it is the least significant",
                        max_bones_per_vertex, vertex_id, bone_info.id, current_weight
                    );
                }
            }
        }
    }
}

void Model::process_materials(aiMesh* mesh, aiScene const* scene, Mesh& new_mesh)
{
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // Attempt to load the materials if they can be found. However PBR materials will need to be manually configured
        // since Assimp doesn't support them Only colour data for the renderer is considered sRGB, all other type of
        // non-colour texture data shouldn't be corrected by the hardware
        new_mesh.material.set_albedo_map(*load_material_texture(material, aiTextureType_DIFFUSE, true));
        new_mesh.material.set_normal_map(*load_material_texture(material, aiTextureType_NORMALS, false));
        new_mesh.material.set_ambient_occlusion_map(*load_material_texture(material, aiTextureType_AMBIENT, false));
        new_mesh.material.set_displacement_map(*load_material_texture(material, aiTextureType_DISPLACEMENT, false));
    }
}

Texture* Model::load_material_texture(aiMaterial* mat, aiTextureType const type, bool const is_srgb)
{
    // Log material constraints are being violated (1 texture per type for the standard shader)
    if (mat->GetTextureCount(type) > 1) {
        Log::warning("Render::Model "
                     "Mesh's default material contains more than 1 texture for the same type, which isn't currently "
                     "supported by the standard shaders");
    }

    // Load the texture of a certain type, assuming there is one
    if (mat->GetTextureCount(type) > 0) {
        aiString str;
        mat->GetTexture(type, 0, &str); // Grab only the first texture (standard shader only supports one texture of
                                        // each type, it doesn't know how you want to do special blending)

        // Assumption made: material stuff is located in the same directory as the model object
        std::string const file_to_search = directory + "/" + std::string(str.C_Str());

        TextureSettings textureSettings;
        textureSettings.is_srgb = is_srgb;
        return Resources::get()->load_2d_texture_async(file_to_search, &textureSettings);
    }

    return nullptr;
}
}