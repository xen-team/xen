#pragma once

namespace xen {
struct TextureSettings {
    // Texture format
    GLenum texture_format = GL_NONE; // If set to GL_NONE, the data format will be used

    /* isSRGB will let the loader know that the texture needs to be "linearlized" before it is sampled in the shaders
     * (ie the texture is in a non liner space) Anything that will be used for color in a renderer should be
     * linearlized. However textures that contain data (Heightfields, normal maps, metallic maps etc.) should not be,
     * thus they are not in SRGB space. Note: If you generate your own data and it is already in linear space (like
     * light probes), be careful */
    bool is_srgb = false;

    // Texture wrapping options
    GLenum texture_wrap_s_mode = GL_REPEAT;
    GLenum texture_wrap_t_mode = GL_REPEAT;
    bool has_border = false;
    Vector4f border_color = Vector4f(1.f);

    // Texture filtering options
    GLenum texture_minification_filter_mode =
        GL_LINEAR_MIPMAP_LINEAR; // Filtering mode when the texture moves further away and multiple texels map to one
                                 // pixel (trilinear for best quality)
    GLenum texture_magnification_filter_mode =
        GL_LINEAR; // Filtering mode when the texture gets closer and multiple pixels map to a single texel (Never needs
                   // to be more than bilinear because that is as accurate as it gets in this sitation)

    float texture_anisotropy_level =
        anisotropic_filtering_level; // Specified independent of texture min and mag filtering, should be a power of 2
                                     // (1.0 means the usual isotropic texture filtering is used which means anisotropic
                                     // filtering isn't used)

    // Mip options
    bool has_mips = true;
    int mip_bias =
        0; // positive means blurrier texture selected, negative means sharper texture which can show texture aliasing
};

class Texture {
private:
    uint id = 0;
    GLenum texture_target = 0;
    Vector2ui extent = Vector2ui(0);

    TextureSettings settings = {};

public:
    Texture() = default;
    Texture(TextureSettings& settings);
    Texture(Texture const& texture); // Copies another texture and its settings
    ~Texture();

    // Generation functions
    void generate_2d_texture(
        Vector2ui const& extent, GLenum data_format, GLenum pixel_data_type = GL_UNSIGNED_BYTE,
        void const* data = nullptr
    );
    void generate_2d_multisample_texture(Vector2ui const& extent);
    void generate_mips(); // Will attempt to generate mipmaps, only works if the texture has already been generated

    void bind(int unit = 0) const;
    void unbind() const;

    // Texture Tuning Functions (Works for pre-generation and post-generation). For post generation you need to bind the
    // texture before calling
    void set_texture_wrap_s(GLenum texture_wrap_mode);
    void set_texture_wrap_t(GLenum texture_wrap_mode);
    void set_has_border(bool has_border);
    void set_border_color(Vector4f const& border_color);
    void set_texture_min_filter(GLenum texture_filter_mode);
    void set_texture_mag_filter(GLenum texture_filter_mode);
    void set_anisotropic_filtering_mode(float texture_anisotropy_level);
    void set_mip_bias(int mip_bias);
    void set_has_mips(bool has_mips);

    // Pre-generation controls only
    void set_texture_settings(TextureSettings&& settings) { this->settings = std::move(settings); }
    void set_texture_format(GLenum format) { settings.texture_format = format; }

    // Don't use this to bind the texture and use it. Call the Bind() function instead
    [[nodiscard]] uint get_id() const { return id; }
    [[nodiscard]] uint get_texture_target() const { return texture_target; }
    [[nodiscard]] bool is_generated() const { return id != 0; }
    [[nodiscard]] uint get_width() const { return extent.x; }
    [[nodiscard]] uint get_height() const { return extent.y; }
    [[nodiscard]] Vector2ui get_extent() const { return extent; }
    [[nodiscard]] TextureSettings const& get_texture_settings() const { return settings; }

    template <class Archive>
    void save(Archive& ar) const
    {
        // ar(colliders, mass, friction, friction_rolling, friction_spinning, linear_factor, angular_factor);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        // ar(colliders, mass, friction, friction_rolling, friction_spinning, linear_factor, angular_factor);
    }

private:
    void apply_texture_settings();
};
}