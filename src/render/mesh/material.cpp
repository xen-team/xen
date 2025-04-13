#include "material.hpp"

#include <render/shader.hpp>
#include <render/texture/texture.hpp>
#include <resource/resources.hpp>

namespace xen {
void Material::set_albedo_map(Texture& texture)
{
#ifdef XEN_DEBUG
    if (!texture.get_texture_settings().is_srgb) {
        Log::error("Render::Material "
                   "Albedo texture isn't set to sRGB space - it won't be properly linearized");
    }
#endif
    albedo_map = &texture;
    albedo_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
}

void Material::set_emission_map(Texture& texture)
{
#ifdef XEN_DEBUG
    if (!texture.get_texture_settings().is_srgb) {
        Log::error("Render::Material "
                   "Emission texture isn't set to sRGB space - it won't be properly linearized");
    }
#endif

    emission_map = &texture;
}

void Material::bind_material_information(Shader& shader)
{
    // Texture unit 0 is reserved for the directional shadowmap
    // Texture unit 1 is reserved for the spotlight shadowmap
    // Texture unit 2 is reserved for the pointlight shadowmap
    // Texture unit 3 is reserved for the irradianceMap used for indirect diffuse IBL
    // Texture unit 4 is reserved for the prefilterMap used for indirect specular IBL
    // Texture unit 5 is reserved for the brdfLUT used for indirect specular IBL
    int current_texture_unit = 6;

    shader.set_uniform("material.albedoColour", static_cast<Vector3f>(albedo_color));

    if (albedo_map && albedo_map->is_generated()) {
        shader.set_uniform("material.texture_albedo", current_texture_unit);
        shader.set_uniform("material.hasAlbedoTexture", true);
        albedo_map->bind(current_texture_unit++);
    }
    else {
        shader.set_uniform("material.hasAlbedoTexture", false);
    }

    shader.set_uniform("material.texture_normal", current_texture_unit);
    if (normal_map && normal_map->is_generated()) {
        normal_map->bind(current_texture_unit++);
    }
    else {
        Resources::get_default_normal_texture()->bind(current_texture_unit++);
    }

    if (metallic_map && metallic_map->is_generated()) {
        shader.set_uniform("material.texture_metallic", current_texture_unit);
        shader.set_uniform("material.hasMetallicTexture", true);
        metallic_map->bind(current_texture_unit++);
    }
    else {
        shader.set_uniform("material.hasMetallicTexture", false);
        shader.set_uniform("material.metallicValue", metallic);
    }

    if (roughness_map && roughness_map->is_generated()) {
        shader.set_uniform("material.texture_roughness", current_texture_unit);
        shader.set_uniform("material.hasRoughnessTexture", true);
        roughness_map->bind(current_texture_unit++);
    }
    else {
        shader.set_uniform("material.hasRoughnessTexture", false);
        shader.set_uniform("material.roughnessValue", roughness);
    }

    shader.set_uniform("material.texture_ao", current_texture_unit);
    if (ambient_occlusion_map && ambient_occlusion_map->is_generated()) {
        ambient_occlusion_map->bind(current_texture_unit++);
    }
    else {
        Resources::get_default_ao_texture()->bind(current_texture_unit++);
    }

    if (displacement_map && displacement_map->is_generated()) {
        shader.set_uniform("hasDisplacement", true);
        shader.set_uniform("minMaxDisplacementSteps", Vector2f(parallax_min_steps, parallax_max_steps));
        shader.set_uniform("parallaxStrength", parallax_strength);
        shader.set_uniform("material.texture_displacement", current_texture_unit);
        displacement_map->bind(current_texture_unit++);
    }
    else {
        shader.set_uniform("has_displacement", false);
    }

    if (emission_map && emission_map->is_generated()) {
        shader.set_uniform("hasEmission", true);
        shader.set_uniform("material.emissionIntensity", emission_intensity);
        shader.set_uniform("material.hasEmissionTexture", true);
        shader.set_uniform("material.texture_emission", current_texture_unit);
        emission_map->bind(current_texture_unit++);
    }
    else if (emission_color.r != 0.0f || emission_color.g != 0.0f || emission_color.b != 0.0f) {
        shader.set_uniform("hasEmission", true);
        shader.set_uniform("material.emissionColour", static_cast<Vector3f>(emission_color));
        shader.set_uniform("material.emissionIntensity", emission_intensity);
        shader.set_uniform("material.hasEmissionTexture", false);
    }
    else {
        shader.set_uniform("hasEmission", false);
        shader.set_uniform("material.hasEmissionTexture", false);
        shader.set_uniform("material.emissionIntensity", 0.0f);
    }
}
}