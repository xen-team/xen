#pragma once

namespace xen {
struct CubemapSettings {
    // Texture format
    GLenum texture_format = GL_NONE; // If set to GL_NONE, the data format will be used

    /* isSRGB will let the loader know that the texture needs to be "linearlized" before it is sampled in the shaders
     * (ie the texture is in a non linear space) Anything that will be used for colour in a renderer should be
     * linearlized. However textures that contain data (Heightfields, normal maps, metallic maps etc.) should not be,
     * thus they are not in sRGB space. Note: If you generate your own data and it is already in linear space (like
     * light probes), be careful */
    bool is_srgb = false;

    // Texture wrapping options
    GLenum texture_wrap_s_mode = GL_CLAMP_TO_EDGE;
    GLenum texture_wrap_t_mode = GL_CLAMP_TO_EDGE;
    GLenum texture_wrap_r_mode = GL_CLAMP_TO_EDGE;

    // Texture filtering options
    GLenum texture_minification_filter_mode =
        GL_LINEAR; // Filtering mode when the texture moves further away and multiple
                   // texels map to one pixel (trilinear for best quality)
    GLenum texture_magnification_filter_mode =
        GL_LINEAR; // Filtering mode when the texture gets closer and multiple pixels map to a single texel (Never needs
                   // to be more than bilinear because that is as accurate as it gets in this sitation)
    float texture_anisotropy_level =
        anisotropic_filtering_level; // Specified independent of texture min and mag filtering, should be a power of 2
                                     // (1.0 means the usual isotropic texture filtering is used which means anisotropic
                                     // filtering isn't used)

    // Mip Settings
    bool has_mips = false;
    int mip_bias =
        0; // positive means blurrier texture selected, negative means sharper texture which can show texture aliasing
};

class Cubemap {
private:
    uint id = 0;
    Vector2ui size = Vector2ui(0);
    uint faces_generated = 0;

    CubemapSettings settings = {};

public:
    Cubemap() = default;
    Cubemap(CubemapSettings& settings);
    ~Cubemap();

    void generate_cubemap_face(GLenum face, Vector2ui const& size, GLenum data_format, uint8_t const* data);
    void
    generate_cubemap_faces(uint count, GLenum face, Vector2ui const& size, GLenum data_format, uint8_t const* data);

    void bind(int unit = 0);
    void unbind();

    void set_cubemap_settings(CubemapSettings settings) { this->settings = settings; }

    [[nodiscard]] uint get_id() const { return id; }
    [[nodiscard]] uint get_face_width() const { return size.x; }
    [[nodiscard]] uint get_face_height() const { return size.y; }

private:
    void generate_cubemap(Vector2ui const& size, GLenum data_format);

    void apply_cubemap_settings();
};
}