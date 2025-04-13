#pragma once

#include <memory>

namespace xen {
class Shader;
class Texture;
class Color;

class Material {
private:
    // Textures will be given precedence if provided over raw values
    Texture* albedo_map = nullptr;
    Texture* normal_map = nullptr;
    Texture* metallic_map = nullptr;
    Texture* roughness_map = nullptr;
    Texture* ambient_occlusion_map = nullptr;
    Texture* displacement_map = nullptr;
    Texture* emission_map = nullptr;

    Color albedo_color = Color(0.894f, 0.023f, 0.992f, 1.0f);
    float metallic = 0.0f;
    float roughness = 0.0f;

    float parallax_strength = 0.07f;
    int parallax_min_steps = parallax_min_steps;
    int parallax_max_steps = parallax_max_steps; // Will need to increase when parallax strength increases

    float emission_intensity = 1.0f;
    Color emission_color = Color(0.0f, 0.0f, 0.0f);

public:
    Material() = default;

    void bind_material_information(Shader& shader);

    void set_albedo_map(Texture& texture);
    void set_normal_map(Texture& texture) { normal_map = &texture; }
    void set_metallic_map(Texture& texture) { metallic_map = &texture; }
    void set_roughness_map(Texture& texture) { roughness_map = &texture; }
    void set_ambient_occlusion_map(Texture& texture) { ambient_occlusion_map = &texture; }
    void set_displacement_map(Texture& texture) { displacement_map = &texture; }
    void set_emission_map(Texture& texture);

    void set_albedo_color(Color const& color) { albedo_color = color; }
    void set_metallic_value(float value) { metallic = value; }
    void set_roughness_value(float value) { roughness = value; }
    void set_displacment_strength(float strength) { parallax_strength = strength; }
    void set_displacement_min_steps(int steps) { parallax_min_steps = steps; }
    void set_displacement_max_steps(int steps) { parallax_max_steps = steps; }
    void set_emission_intensity(float intensity) { emission_intensity = intensity; }
    void set_emission_color(Color const& color) { emission_color = color; }

    [[nodiscard]] Texture const* get_albedo_map() const { return albedo_map; }
    [[nodiscard]] Texture const* get_normal_map() const { return normal_map; }
    [[nodiscard]] Texture const* get_metallic_map() const { return metallic_map; }
    [[nodiscard]] Texture const* get_roughness_map() const { return roughness_map; }
    [[nodiscard]] Texture const* get_ambient_occlusion_map() const { return ambient_occlusion_map; }
    [[nodiscard]] Texture const* get_displacement_map() const { return displacement_map; }
    [[nodiscard]] Texture const* get_emission_map() const { return emission_map; }

    [[nodiscard]] Color get_albedo_color() const { return albedo_color; }
    [[nodiscard]] float get_metallic_value() const { return metallic; }
    [[nodiscard]] float get_roughness_value() const { return roughness; }
    [[nodiscard]] float get_displacement_strength() const { return parallax_strength; }
    [[nodiscard]] int get_displacement_min_steps() const { return parallax_min_steps; }
    [[nodiscard]] int get_displacement_max_steps() const { return parallax_max_steps; }
    [[nodiscard]] float get_emission_intensity() const { return emission_intensity; }
    [[nodiscard]] Color get_emission_color() const { return emission_color; }

    template <class Archive>
    void save(Archive& ar) const
    {
        // ar(vao, vbo, ibo, positions, uvs, normals, tangents, bitangents, bone_data, indices, buffer_data,
        //    is_interleaved, buffer_component_count);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        // ar(vao, vbo, ibo, positions, uvs, normals, tangents, bitangents, bone_data, indices, buffer_data,
        //    is_interleaved, buffer_component_count);
    }
};
}