#pragma once

#include "platform/opengl/image/image.hpp"
#include "math/vector4.hpp"

#include <magic_enum.hpp>

namespace xen {
enum class TextureFormat : uint8_t {
    R,
    R16,
    RG,
    RG16,
    R16F,
    R32F,
    RG16F,
    RG32F,
    RGB,
    RGBA,
    RGB16,
    RGB16F,
    RGBA16,
    RGBA16F,
    RGB32F,
    RGBA32F,
    DEPTH,
    DEPTH32F
};

enum class TextureWrap : uint8_t {
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER,
    MIRRORED_REPEAT,
    REPEAT,
};

using TextureHandle = uint32_t;

class Texture {
public:
    using RawData = uint8_t;
    using RawDataPointer = RawData*;

private:
    using BindableId = unsigned int;

    std::string filepath;
    size_t width = 0;
    size_t height = 0;
    BindableId id = 0;
    TextureHandle active_id = 0;
    unsigned int texture_type = 0;
    TextureFormat format = TextureFormat::RGB;
    uint8_t samples = 0;

public:
    Texture();
    Texture(Texture const&) = delete;
    Texture(Texture&& texture) noexcept;
    Texture& operator=(Texture const& texture) = delete;
    Texture& operator=(Texture&& texture) noexcept;
    ~Texture();

    Texture(std::string_view filepath, TextureFormat format);

    void bind();
    void bind(TextureHandle id);

    void unbind();

    void load(std::string_view filepath, TextureFormat format);

    void load(RawDataPointer data, int width, int height, int channels, TextureFormat format = TextureFormat::RGB);
    void load(Image const& image, TextureFormat format = TextureFormat::RGB);

    void load_depth(int width, int height, TextureFormat format = TextureFormat::DEPTH);

    void generate_mipmaps();

    [[nodiscard]] bool is_multisampled() const;
    [[nodiscard]] bool is_depth_only() const;

    void set_max_lod(size_t lod);
    void set_min_lod(size_t lod);

    void set_border_color(Vector4f const& color);
    void set_wrap_type(TextureWrap wrap);

    [[nodiscard]] TextureHandle get_bound_id() const { return active_id; }
    [[nodiscard]] BindableId get_native_handle() const { return id; };
    [[nodiscard]] unsigned int get_texture_type() const { return texture_type; }
    [[nodiscard]] TextureFormat get_format() const { return format; }
    [[nodiscard]] size_t get_width() const { return width; }
    [[nodiscard]] size_t get_height() const { return height; }
    [[nodiscard]] std::string_view get_file_path() const { return filepath; }

    [[nodiscard]] size_t get_sample_count() const;
    [[nodiscard]] size_t get_pixel_size() const;
    [[nodiscard]] size_t get_channel_count() const;
    [[nodiscard]] size_t get_max_texture_lod() const;

    [[nodiscard]] TextureWrap get_wrap_type();
    [[nodiscard]] Vector4f get_border_color();
    [[nodiscard]] Image get_raw_texture_data();
    // void SetInternalEngineTag(MxString const& tag);
    // bool IsInternalEngineResource() const;

private:
    void free();
};
}