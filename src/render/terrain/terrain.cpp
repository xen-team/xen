#include "terrain.hpp"

#include <cstddef>
#include <render/mesh/mesh.hpp>
#include <render/renderer/gl_cache.hpp>
#include <render/render.hpp>
#include <render/shader.hpp>
#include <resource/resources.hpp>
#include <stb_image.h>

namespace xen {
Terrain::Terrain() : gl_cache(&GLCache::get()), model{Matrix4(1.f).translate(position)}
{
    Resources& resources = *Resources::get();

    TextureSettings srgb_texture_settings;
    srgb_texture_settings.is_srgb = true;

    textures[0] = resources.load_2d_texture_async("res/terrain/grass/grass_albedo.tga", &srgb_texture_settings);
    textures[1] = resources.load_2d_texture_async("res/terrain/dirt/dirt_albedo.tga", &srgb_texture_settings);
    textures[2] = resources.load_2d_texture_async("res/terrain/branches/branches_albedo.tga", &srgb_texture_settings);
    textures[3] = resources.load_2d_texture_async("res/terrain/rock/rock_albedo.tga", &srgb_texture_settings);

    textures[4] = resources.load_2d_texture_async("res/terrain/grass/grass_normal.tga");
    textures[5] = resources.load_2d_texture_async("res/terrain/dirt/dirt_normal.tga");
    textures[6] = resources.load_2d_texture_async("res/terrain/branches/branches_normal.tga");
    textures[7] = resources.load_2d_texture_async("res/terrain/rock/rock_normal.tga");

    // We do not want these texture treated as one channel so store it as RGB
    TextureSettings texture_settings;
    texture_settings.texture_format = GL_RGB;

    textures[8] = resources.load_2d_texture_async("res/terrain/grass/grass_roughness.tga", &texture_settings);
    textures[9] = resources.load_2d_texture_async("res/terrain/dirt/dirt_roughness.tga", &texture_settings);
    textures[10] = resources.load_2d_texture_async("res/terrain/branches/branches_roughness.tga", &texture_settings);
    textures[11] = resources.load_2d_texture_async("res/terrain/rock/rock_roughness.tga", &texture_settings);

    textures[12] = resources.load_2d_texture_async("res/terrain/grass/grass_metallic.tga", &texture_settings);
    textures[13] = resources.load_2d_texture_async("res/terrain/dirt/dirt_metallic.tga", &texture_settings);
    textures[14] = resources.load_2d_texture_async("res/terrain/branches/branches_metallic.tga", &texture_settings);
    textures[15] = resources.load_2d_texture_async("res/terrain/rock/rock_metallic.tga", &texture_settings);

    textures[16] = resources.load_2d_texture_async("res/terrain/grass/grass_ao.tga", &texture_settings);
    textures[17] = resources.load_2d_texture_async("res/terrain/dirt/dirt_ao.tga", &texture_settings);
    textures[18] = resources.load_2d_texture_async("res/terrain/branches/branches_ao.tga", &texture_settings);
    textures[19] = resources.load_2d_texture_async("res/terrain/rock/rock_ao.tga", &texture_settings);

    textures[20] = resources.load_2d_texture_async("res/terrain/blend_map.tga", &texture_settings);
}

void Terrain::from_texture(std::string_view texture_path)
{
    if (is_loaded()) {
        Log::info("Render::Terrain "
                  "Unloading the old terrain in order to load a new terrain");
    }

    Log::vinfo(
        "Render::Terrain "
        "Loading the terrain from texture path: ",
        texture_path
    );

    // Height map
    int map_width, map_height;
    uint8_t* height_map_image_data = stbi_load("res/terrain/height_map.png", &map_width, &map_height, nullptr, 1);
    if (map_width != map_height) {
        Log::error("Render::Terrain "
                   "Can't use a heightmap with a different width and height for the terrain");
        return;
    }

    // Terrain information
    texture_tiling_amount = 64;
    heightfield_texture_size = map_width;
    side_vertex_count = static_cast<uint>(static_cast<float>(map_width) * 0.25f);
    terrain_size_xz = 512.f;
    terrain_size_y = 100.f;
    space_between_vertices = terrain_size_xz / static_cast<float>(side_vertex_count);
    terrain_to_heightfield_texture_conversion = 1.0f / (terrain_size_xz / static_cast<float>(heightfield_texture_size));

    auto const side_vertex_count_squared = side_vertex_count * side_vertex_count;
    uint const side_vertex_count_minus_one = side_vertex_count - 1;

    // Requirements to generate a mesh
    std::vector<Vector3f> positions;
    std::vector<Vector2f> uvs;
    std::vector<Vector3f> normals;
    std::vector<Vector3f> tangents(side_vertex_count_squared, Vector3f(0.0f, 0.0f, 0.0f));
    std::vector<Vector3f> bitangents(side_vertex_count_squared, Vector3f(0.0f, 0.0f, 0.0f));
    std::vector<uint> indices;

    positions.reserve(side_vertex_count_squared);
    uvs.reserve(side_vertex_count_squared);
    normals.reserve(side_vertex_count_squared);
    indices.reserve(static_cast<size_t>(side_vertex_count_minus_one * side_vertex_count_minus_one) * 6);

    // Vertex generation
    for (uint z = 0; z < side_vertex_count; z++) {
        for (uint x = 0; x < side_vertex_count; x++) {
            Vector2f const position_xz(
                static_cast<float>(x) * space_between_vertices, static_cast<float>(z) * space_between_vertices
            );

            positions.emplace_back(
                position_xz.x, sample_heightfield_bilinear(position_xz.x, position_xz.y, height_map_image_data),
                position_xz.y
            );

            uvs.emplace_back(
                static_cast<float>(x) / static_cast<float>(side_vertex_count_minus_one),
                static_cast<float>(z) / static_cast<float>(side_vertex_count_minus_one)
            );

            normals.emplace_back(calculate_normal(position_xz.x, position_xz.y, height_map_image_data));
        }
    }

    stbi_image_free(height_map_image_data);

    // Indices generation (ccw winding order for consistency which will allow back face culling)
    for (uint height = 0; height < side_vertex_count_minus_one; height++) {
        for (uint width = 0; width < side_vertex_count_minus_one; width++) {

            uint const index_tl = width + (height * side_vertex_count);
            uint const index_tr = 1 + width + (height * side_vertex_count);
            uint const index_bl = side_vertex_count + width + (height * side_vertex_count);
            uint const index_br = 1 + side_vertex_count + width + (height * side_vertex_count);

            // Triangle 1
            indices.emplace_back(index_tl);
            indices.emplace_back(index_br);
            indices.emplace_back(index_tr);

            // Triangle 2
            indices.emplace_back(index_tl);
            indices.emplace_back(index_bl);
            indices.emplace_back(index_br);

            // Triangle 1 tangent + bitangent calculations
            Vector3f const& v0 = positions[index_tl];
            Vector3f const& v1 = positions[index_br];
            Vector3f const& v2 = positions[index_tr];
            Vector2f const& uv0 = uvs[index_tl];
            Vector2f const& uv1 = uvs[index_br];
            Vector2f const& uv2 = uvs[index_tr];
            Vector3f const delta_pos1 = v1 - v0;
            Vector3f const delta_pos2 = v2 - v0;
            Vector2f const delta_uv1 = uv1 - uv0;
            Vector2f const delta_uv2 = uv2 - uv0;

            float const r = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x);
            Vector3f const tangent = (delta_pos1 * delta_uv2.y - delta_pos2 * delta_uv1.y) * r;

            tangents[index_tl] += tangent;
            tangents[index_br] += tangent;
            tangents[index_tr] += tangent;

            tangents[index_tl] += tangent;
            tangents[index_bl] += tangent;
            tangents[index_br] += tangent;
        }
    }

    // Gram-Schmidt Process for fixing up the tangent vector and calculating the bitangent
    for (size_t i = 0; i < tangents.size(); i++) {
        Vector3f const& normal = normals[i];
        Vector3f tangent = tangents[i].normalize();

        tangent = (tangent - tangent.dot(normal) * normal).normalize();
        Vector3f const bitangent = normal.cross(tangent).normalize();

        tangents[i] = tangent;
        bitangents[i] = bitangent;
    }

    mesh = std::make_unique<Mesh>(
        std::move(positions), std::move(uvs), std::move(normals), std::move(tangents), std::move(bitangents),
        std::vector<VertexBoneData>(), std::move(indices)
    );

    mesh->load_data(true);
    mesh->generate_gpu_data();
}

void Terrain::draw(Shader& shader, RenderPassType const pass) const
{
    // Texture unit 0 is reserved for the directional light shadowmap
    // Texture unit 1 is reserved for the spot light shadowmap
    // Texture unit 2 is reserved for the point light shadowmap
    if (pass == RenderPassType::MaterialRequired) {
        int current_texture_unit = 3;

        textures[0]->bind(current_texture_unit);
        shader.set_uniform("material.texture_albedo1", current_texture_unit++);
        textures[1]->bind(current_texture_unit);
        shader.set_uniform("material.texture_albedo2", current_texture_unit++);
        textures[2]->bind(current_texture_unit);
        shader.set_uniform("material.texture_albedo3", current_texture_unit++);
        textures[3]->bind(current_texture_unit);
        shader.set_uniform("material.texture_albedo4", current_texture_unit++);

        textures[4]->bind(current_texture_unit);
        shader.set_uniform("material.texture_normal1", current_texture_unit++);
        textures[5]->bind(current_texture_unit);
        shader.set_uniform("material.texture_normal2", current_texture_unit++);
        textures[6]->bind(current_texture_unit);
        shader.set_uniform("material.texture_normal3", current_texture_unit++);
        textures[7]->bind(current_texture_unit);
        shader.set_uniform("material.texture_normal4", current_texture_unit++);

        textures[8]->bind(current_texture_unit);
        shader.set_uniform("material.texture_roughness1", current_texture_unit++);
        textures[9]->bind(current_texture_unit);
        shader.set_uniform("material.texture_roughness2", current_texture_unit++);
        textures[10]->bind(current_texture_unit);
        shader.set_uniform("material.texture_roughness3", current_texture_unit++);
        textures[11]->bind(current_texture_unit);
        shader.set_uniform("material.texture_roughness4", current_texture_unit++);

        textures[12]->bind(current_texture_unit);
        shader.set_uniform("material.texture_metallic1", current_texture_unit++);
        textures[13]->bind(current_texture_unit);
        shader.set_uniform("material.texture_metallic2", current_texture_unit++);
        textures[14]->bind(current_texture_unit);
        shader.set_uniform("material.texture_metallic3", current_texture_unit++);
        textures[15]->bind(current_texture_unit);
        shader.set_uniform("material.texture_metallic4", current_texture_unit++);

        textures[16]->bind(current_texture_unit);
        shader.set_uniform("material.texture_AO1", current_texture_unit++);
        textures[17]->bind(current_texture_unit);
        shader.set_uniform("material.texture_AO2", current_texture_unit++);
        textures[18]->bind(current_texture_unit);
        shader.set_uniform("material.texture_AO3", current_texture_unit++);
        textures[19]->bind(current_texture_unit);
        shader.set_uniform("material.texture_AO4", current_texture_unit++);

        textures[20]->bind(current_texture_unit);
        shader.set_uniform("material.blendmap", current_texture_unit++);

        Matrix3 const normal_matrix = model.inverse().transpose();
        shader.set_uniform("normalMatrix", normal_matrix);

        shader.set_uniform("material.tilingAmount", texture_tiling_amount);
    }

    // Only set normal matrix for non shadowmap pass
    shader.set_uniform("model", model);

    gl_cache->set_depth_test(true);
    gl_cache->set_depth_func(GL_LESS);
    gl_cache->set_blend(false);
    gl_cache->set_face_cull(true);
    gl_cache->set_cull_face(GL_BACK);
    mesh->draw();
}

// Bilinear filtering for the terrain's normal
Vector3f
Terrain::calculate_normal(float const world_pos_x, float const world_pos_z, uint8_t const* height_map_data) const
{
    float const height_r =
        sample_heightfield_nearest(world_pos_x + space_between_vertices * 2, world_pos_z, height_map_data);
    float const height_l =
        sample_heightfield_nearest(world_pos_x - space_between_vertices * 2, world_pos_z, height_map_data);
    float const height_u =
        sample_heightfield_nearest(world_pos_x, world_pos_z + space_between_vertices * 2, height_map_data);
    float const height_d =
        sample_heightfield_nearest(world_pos_x, world_pos_z - space_between_vertices * 2, height_map_data);

    Vector3f normal(height_l - height_r, 2.0f, height_d - height_u);
    return normal.normalize();
}

float Terrain::sample_heightfield_bilinear(
    float const world_pos_x, float const world_pos_z, uint8_t const* height_map_data
) const
{
    // Calculate weights
    Vector2f const weights_xz = Vector2f(world_pos_x / space_between_vertices, world_pos_z / space_between_vertices);
    float const x_frac = weights_xz.x - weights_xz.x;
    float const z_frac = weights_xz.y - weights_xz.y;

    // Get the values that should be lerped between
    float const top_left = sample_heightfield_nearest(world_pos_x, world_pos_z, height_map_data);
    float const top_right =
        sample_heightfield_nearest(world_pos_x + space_between_vertices, world_pos_z, height_map_data);
    float const bottom_left =
        sample_heightfield_nearest(world_pos_x, world_pos_z + space_between_vertices, height_map_data);
    float const bottom_right = sample_heightfield_nearest(
        world_pos_x + space_between_vertices, world_pos_z + space_between_vertices, height_map_data
    );

    // Do the bilinear filtering
    float const terrain_height =
        Math::lerp(Math::lerp(top_left, top_right, x_frac), Math::lerp(bottom_left, bottom_right, x_frac), z_frac);

    return terrain_height;
}

float Terrain::sample_heightfield_nearest(
    float const world_pos_x, float const world_pos_z, uint8_t const* height_map_data
) const
{
    // Get the xz coordinates of the index after cutting off the decimal
    Vector2f const terrain_xz = Vector2f(
        std::clamp(
            world_pos_x * terrain_to_heightfield_texture_conversion, 0.0f,
            terrain_to_heightfield_texture_conversion - 0.0f
        ),
        std::clamp(
            world_pos_z * terrain_to_heightfield_texture_conversion, 0.0f,
            terrain_to_heightfield_texture_conversion - 1.0f
        )
    );

    // _normalize height to [0, 1] then multiply it by the terrain's Y scale
    return static_cast<float>(
               height_map_data
                   [static_cast<uint>(terrain_xz.x) + (static_cast<uint>(terrain_xz.y) * heightfield_texture_size)]
           ) /
           255.0f * terrain_size_y;
}
}