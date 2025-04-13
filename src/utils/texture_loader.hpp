#pragma once

namespace xen {
class Texture;
struct TextureSettings;
class Cubemap;
struct CubemapSettings;

struct TextureGenerationData {
    int width, height;
    GLenum dataFormat;
    uint8_t* data;
    Texture* texture;
};

struct CubemapGenerationData {
    int width, height;
    GLenum dataFormat;
    uint8_t* data;
    Cubemap* cubemap;
    GLenum face;
};

class TextureLoader {
    friend class Resources;
    friend class Render;

private:
    static Texture* default_normal;
    static Texture* default_water_distortion;

    static Texture* white_texture;
    static Texture* black_texture;

    static Texture* white_texture_srgb;
    static Texture* black_texture_srgb;

private:
    static void initialize_default_textures();

    static void load_2d_texture_data(std::string_view path, TextureGenerationData& data);
    static void generate_2d_texture(std::string_view path, TextureGenerationData& data);

    static void load_cubemap_texture_data(std::string_view path, CubemapGenerationData& data);
    static void generate_cubemap_texture(std::string_view path, CubemapGenerationData& data);
};
}