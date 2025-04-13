#include "texture_loader.hpp"

#include <render/texture/texture.hpp>
#include <render/texture/cubemap.hpp>
#include <resource/resources.hpp>

#include <stb_image.h>

namespace xen {
Texture* TextureLoader::default_normal;
Texture* TextureLoader::default_water_distortion;
Texture* TextureLoader::white_texture;
Texture* TextureLoader::black_texture;
Texture* TextureLoader::white_texture_srgb;
Texture* TextureLoader::black_texture_srgb;

void TextureLoader::load_2d_texture_data(std::string_view path, TextureGenerationData& data)
{
    // Load the texture data from file
    int num_components;
    data.data = stbi_load(std::filesystem::path(path).c_str(), &data.width, &data.height, &num_components, 0);
    if (!data.data) {
        Log::verror(
            "TextureLoader "
            "Failed to load texture path: {}",
            path
        );
        stbi_image_free(data.data);
        return;
    }

    switch (num_components) {
    case 1:
        data.dataFormat = GL_RED;
        break;
    case 3:
        data.dataFormat = GL_RGB;
        break;
    case 4:
        data.dataFormat = GL_RGBA;
        break;
    }
}

void TextureLoader::generate_2d_texture(std::string_view path, TextureGenerationData& data)
{
    data.texture->generate_2d_texture(Vector2ui(data.width, data.height), data.dataFormat, GL_UNSIGNED_BYTE, data.data);
    stbi_image_free(data.data);
}

void TextureLoader::load_cubemap_texture_data(std::string_view path, CubemapGenerationData& data)
{
    // Load the cubemap data from file
    int num_components;
    data.data = stbi_load(path.data(), &data.width, &data.height, &num_components, 0);
    if (!data.data) {
        Log::verror(
            "TextureLoader "
            "Failed to load cubemap face: {}, at path: {} - Reason: {}",
            data.face, path, stbi_failure_reason()
        );
        stbi_image_free(data.data);
        return;
    }

    switch (num_components) {
    case 1:
        data.dataFormat = GL_RED;
        break;
    case 3:
        data.dataFormat = GL_RGB;
        break;
    case 4:
        data.dataFormat = GL_RGBA;
        break;
    }
}

void TextureLoader::generate_cubemap_texture(std::string_view path, CubemapGenerationData& data)
{
    data.cubemap->generate_cubemap_face(data.face, Vector2ui(data.width, data.height), data.dataFormat, data.data);
    stbi_image_free(data.data);
}

void TextureLoader::initialize_default_textures()
{
    // Setup texture and minimal filtering because they are 1x1 textures so they require none
    TextureSettings settings;
    settings.texture_anisotropy_level = 1.0f;
    settings.texture_minification_filter_mode = GL_NEAREST;
    settings.texture_magnification_filter_mode = GL_NEAREST;

    auto* resource_manager = Resources::get();

    default_normal = resource_manager->load_2d_texture("res/textures/default/default_normal.png", &settings);
    default_water_distortion =
        resource_manager->load_2d_texture("res/textures/default/default_water_distortion.png", &settings);
    white_texture = resource_manager->load_2d_texture("res/textures/default/white.png", &settings);
    black_texture = resource_manager->load_2d_texture("res/textures/default/black.png", &settings);

    TextureSettings srgb_settings;
    srgb_settings.texture_anisotropy_level = 1.0f;
    srgb_settings.texture_minification_filter_mode = GL_NEAREST;
    srgb_settings.texture_magnification_filter_mode = GL_NEAREST;
    srgb_settings.is_srgb = true;

    white_texture_srgb = resource_manager->load_2d_texture("res/textures/default/white_srgb.png", &srgb_settings);
    black_texture_srgb = resource_manager->load_2d_texture("res/textures/default/black_srgb.png", &srgb_settings);
}
}