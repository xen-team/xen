#pragma once

#include <render/renderer/renderpass/render_pass_type.hpp>

namespace xen {
class Shader;
class Mesh;
class GLCache;

class Terrain {
private:
    GLCache* gl_cache;

    // Tweakable Terrain Variables
    float texture_tiling_amount;
    float terrain_size_xz, terrain_size_y;
    uint side_vertex_count;

    // Non-Tweakable Terrain Varialbes
    float space_between_vertices;
    float terrain_to_heightfield_texture_conversion;
    uint heightfield_texture_size;

    Matrix4 model;
    Vector3f position = Vector3f(0.f);
    std::unique_ptr<Mesh> mesh;
    std::array<Texture*, 21> textures; // Represents all the textures supported by the terrain's texture splatting
                                       // (rgba and the default value)

public:
    Terrain();

    void from_texture(std::string_view texture_path);

    void draw(Shader& shader, RenderPassType pass) const;

    void set_position(Vector3f const& position)
    {
        this->position = position;
        model = Matrix4(1.f).translate(position);
    }

    [[nodiscard]] Vector3f const& get_position() const { return position; }
    [[nodiscard]] bool is_loaded() { return mesh != nullptr; }

private:
    Vector3f calculate_normal(float world_pos_x, float world_pos_z, uint8_t const* height_map_data) const;

    float sample_heightfield_bilinear(float world_pos_x, float world_pos_z, uint8_t const* height_map_data) const;
    float sample_heightfield_nearest(float world_pos_x, float world_pos_z, uint8_t const* height_map_data) const;
};
}