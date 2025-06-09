#include "texture.hpp"

#include <data/image.hpp>
#if defined(USE_OPENGL_ES)
#include <render/platform/framebuffer.hpp>
#endif
#include <render/renderer.hpp>

#include <tracy/Tracy.hpp>

namespace xen {
namespace {
inline TextureFormat recover_format(TextureColorspace colorspace)
{
    switch (colorspace) {
    case TextureColorspace::INVALID:
    default:
        break;

    case TextureColorspace::GRAY:
        return TextureFormat::RED;

    case TextureColorspace::RG:
        return TextureFormat::RG;

    case TextureColorspace::RGB:
    case TextureColorspace::SRGB:
        return TextureFormat::RGB;

    case TextureColorspace::RGBA:
    case TextureColorspace::SRGBA:
        return TextureFormat::RGBA;

    case TextureColorspace::DEPTH:
        return TextureFormat::DEPTH;
    }

    throw std::invalid_argument("Error: Invalid texture colorspace");
}

inline TextureInternalFormat recover_internal_format(TextureColorspace colorspace, TextureDataType data_type)
{
    switch (colorspace) {
    case TextureColorspace::INVALID:
    default:
        break;

    case TextureColorspace::GRAY:
        return (
            data_type == TextureDataType::BYTE ?
                TextureInternalFormat::R8 :
                (data_type == TextureDataType::FLOAT16 ? TextureInternalFormat::R16F : TextureInternalFormat::R32F)
        );

    case TextureColorspace::RG:
        return (
            data_type == TextureDataType::BYTE ?
                TextureInternalFormat::RG8 :
                (data_type == TextureDataType::FLOAT16 ? TextureInternalFormat::RG16F : TextureInternalFormat::RG32F)
        );

    case TextureColorspace::RGB:
        return (
            data_type == TextureDataType::BYTE ?
                TextureInternalFormat::RGB8 :
                (data_type == TextureDataType::FLOAT16 ? TextureInternalFormat::RGB16F : TextureInternalFormat::RGB32F)
        );

    case TextureColorspace::RGBA:
        return (
            data_type == TextureDataType::BYTE ?
                TextureInternalFormat::RGBA8 :
                (data_type == TextureDataType::FLOAT16 ? TextureInternalFormat::RGBA16F : TextureInternalFormat::RGBA32F
                )
        );

        // Floating-point sRGB(A) textures are not treated as sRGB, which is necessarily an integer format; they are
        // therefore interpreted as floating-point RGB(A)

    case TextureColorspace::SRGB:
        return (
            data_type == TextureDataType::BYTE ?
                TextureInternalFormat::SRGB8 :
                (data_type == TextureDataType::FLOAT16 ? TextureInternalFormat::RGB16F : TextureInternalFormat::RGB32F)
        );

    case TextureColorspace::SRGBA:
        return (
            data_type == TextureDataType::BYTE ?
                TextureInternalFormat::SRGBA8 :
                (data_type == TextureDataType::FLOAT16 ? TextureInternalFormat::RGBA16F : TextureInternalFormat::RGBA32F
                )
        );

    case TextureColorspace::DEPTH:
        return TextureInternalFormat::DEPTH32F;
    }

    throw std::invalid_argument("[Texture] Invalid texture colorspace to recover the internal format from");
}

inline TextureParamValue recover_param(TextureFilter filter)
{
    switch (filter) {
    case TextureFilter::NEAREST:
        return TextureParamValue::NEAREST;

    case TextureFilter::LINEAR:
        return TextureParamValue::LINEAR;
    }

    throw std::invalid_argument("Error: Invalid texture filter");
}

inline TextureParamValue recover_param(TextureFilter filter, TextureFilter mipmap_filter)
{
    switch (filter) {
    case TextureFilter::NEAREST:
        return (
            mipmap_filter == TextureFilter::NEAREST ? TextureParamValue::NEAREST_MIPMAP_NEAREST :
                                                      TextureParamValue::NEAREST_MIPMAP_LINEAR
        );

    case TextureFilter::LINEAR:
        return (
            mipmap_filter == TextureFilter::NEAREST ? TextureParamValue::LINEAR_MIPMAP_NEAREST :
                                                      TextureParamValue::LINEAR_MIPMAP_LINEAR
        );
    }

    throw std::invalid_argument("Error: Invalid texture filter");
}

inline TextureParamValue recover_param(TextureWrapping wrapping)
{
    switch (wrapping) {
    case TextureWrapping::REPEAT:
        return TextureParamValue::REPEAT;

    case TextureWrapping::CLAMP:
        return TextureParamValue::CLAMP_TO_EDGE;
    }

    throw std::invalid_argument("Error: Invalid texture wrapping");
}

inline TextureColorspace recover_colorspace(ImageColorspace image_colorspace, bool should_use_srgb)
{
    auto texture_colorspace = static_cast<TextureColorspace>(image_colorspace);

    if (should_use_srgb) {
        if (texture_colorspace == TextureColorspace::RGB) {
            texture_colorspace = TextureColorspace::SRGB;
        }
        else if (texture_colorspace == TextureColorspace::RGBA) {
            texture_colorspace = TextureColorspace::SRGBA;
        }
    }

    return texture_colorspace;
}
}

void Texture::bind() const
{
    Renderer::bind_texture(type, index);
}

void Texture::unbind() const
{
    Renderer::unbind_texture(type);
}

void Texture::set_filter(TextureFilter minify, TextureFilter magnify) const
{
    ZoneScopedN("Texture::set_filter");

    bind();
    Renderer::set_texture_parameter(type, TextureParam::MINIFY_FILTER, recover_param(minify));
    Renderer::set_texture_parameter(type, TextureParam::MAGNIFY_FILTER, recover_param(magnify));
    unbind();
}

void Texture::set_filter(TextureFilter minify, TextureFilter mipmapMinify, TextureFilter magnify) const
{
    ZoneScopedN("Texture::set_filter");

    bind();
    Renderer::set_texture_parameter(type, TextureParam::MINIFY_FILTER, recover_param(minify, mipmapMinify));
    Renderer::set_texture_parameter(type, TextureParam::MAGNIFY_FILTER, recover_param(magnify));
    unbind();
}

void Texture::set_wrapping(TextureWrapping wrapping) const
{
    ZoneScopedN("Texture::set_wrapping");

    TextureParamValue const value = recover_param(wrapping);

    bind();
    Renderer::set_texture_parameter(type, TextureParam::WRAP_S, value);
    Renderer::set_texture_parameter(type, TextureParam::WRAP_T, value);
    Renderer::set_texture_parameter(type, TextureParam::WRAP_R, value);
    unbind();
}

void Texture::set_colorspace(TextureColorspace colorspace)
{
    set_colorspace(
        colorspace, (colorspace == TextureColorspace::DEPTH ? TextureDataType::FLOAT32 : TextureDataType::BYTE)
    );
}

void Texture::set_colorspace(TextureColorspace colorspace, TextureDataType data_type)
{
    Log::rt_assert(
        colorspace != TextureColorspace::DEPTH || data_type == TextureDataType::FLOAT32,
        "Error: A depth texture must have a 32-bit floating-point data type."
    );
    Log::rt_assert(
        colorspace != TextureColorspace::DEPTH || type != TextureType::TEXTURE_3D,
        "Error: A depth texture cannot be three-dimensional."
    );

    ZoneScopedN("Texture::set_colorspace");

    this->colorspace = colorspace;
    this->data_type = data_type;

    load();

    if (colorspace == TextureColorspace::DEPTH) {
        set_filter(TextureFilter::NEAREST);
    }
}

Texture::~Texture()
{
    ZoneScopedN("Texture::~Texture");

    if (!index.is_valid()) {
        return;
    }

    Log::debug("[Texture] Destroying (ID: " + std::to_string(index) + ")...");
    Renderer::delete_texture(index);
    Log::debug("[Texture] Destroyed");
}

Texture::Texture(TextureType type) : type{type}
{
    ZoneScopedN("Texture::Texture");

    Log::debug("[Texture] Creating...");
    Renderer::generate_texture(index);
    Log::debug("[Texture] Created (ID: " + std::to_string(index) + ")");

    set_filter(TextureFilter::LINEAR);
    set_wrapping(TextureWrapping::CLAMP);
}

void Texture::set_loaded_parameters(bool create_mipmaps) const
{
    ZoneScopedN("Texture::set_loaded_parameters");

    if (colorspace == TextureColorspace::GRAY || colorspace == TextureColorspace::RG) {
        Renderer::set_texture_parameter(type, TextureParam::SWIZZLE_R, static_cast<int>(TextureFormat::RED));
        Renderer::set_texture_parameter(type, TextureParam::SWIZZLE_G, static_cast<int>(TextureFormat::RED));
        Renderer::set_texture_parameter(type, TextureParam::SWIZZLE_B, static_cast<int>(TextureFormat::RED));
        Renderer::set_texture_parameter(
            type, TextureParam::SWIZZLE_A,
            (colorspace == TextureColorspace::RG ? static_cast<int>(TextureFormat::GREEN) : 1)
        );
    }

    if (create_mipmaps
#if defined(USE_WEBGL)
        // WebGL doesn't seem to support mipmap generation for sRGB textures
        && colorspace != TextureColorspace::SRGB && colorspace != TextureColorspace::SRGBA
#endif
    ) {
        generate_mipmaps();
        set_filter(TextureFilter::LINEAR, TextureFilter::LINEAR, TextureFilter::LINEAR);
    }
    else {
        set_filter(TextureFilter::LINEAR);
    }

    set_wrapping(TextureWrapping::REPEAT);
}

void Texture::generate_mipmaps() const
{
    ZoneScopedN("Texture::generate_mipmaps");

    bind();
    Renderer::generate_mipmap(type);
    unbind();
}

#if !defined(USE_OPENGL_ES)
Texture1D::Texture1D() : Texture(TextureType::TEXTURE_1D) {}

Texture1D::Texture1D(uint32_t width, TextureColorspace colorspace, TextureDataType data_type) :
    Texture1D(colorspace, data_type)
{
    resize(width);
}

Texture1D::Texture1D(Color const& color, uint32_t width) : Texture1D()
{
    this->width = width;

    fill(color);
}

void Texture1D::resize(uint32_t width)
{
    this->width = width;

    load();
}

void Texture1D::fill(Color const& color)
{
    ZoneScopedN("Texture1D::fill");

    colorspace = TextureColorspace::RGB;
    data_type = TextureDataType::BYTE;

    // MARK
    std::vector<Vector3ub> const values(width * 3, Vector3ub(color.r * 255.f, color.g * 255.f, color.b * 255.f));

    bind();
    Renderer::send_image_data_1d(
        TextureType::TEXTURE_1D, 0, TextureInternalFormat::RGB, width, TextureFormat::RGB, PixelDataType::UBYTE,
        values.data()
    );
    unbind();
}

void Texture1D::load() const
{
    ZoneScopedN("Texture1D::load");

    if (colorspace == TextureColorspace::INVALID) {
        return; // No colorspace has been set yet, the texture can't be loaded
    }

    bind();
    Renderer::send_image_data_1d(
        TextureType::TEXTURE_1D, 0, recover_internal_format(colorspace, data_type), width, recover_format(colorspace),
        (data_type == TextureDataType::BYTE ? PixelDataType::UBYTE : PixelDataType::FLOAT), nullptr
    );
    unbind();
}
#endif

Texture2D::Texture2D() : Texture(TextureType::TEXTURE_2D) {}

Texture2D::Texture2D(Vector2ui const& size, TextureColorspace colorspace, TextureDataType data_type) :
    Texture2D(colorspace, data_type)
{
    resize(size);
}

Texture2D::Texture2D(Color const& color, Vector2ui size) : Texture2D()
{
    this->size = std::move(size);

    fill(color);
}

void Texture2D::resize(Vector2ui const& size)
{
    this->size = size;

    load();
}

void Texture2D::load(Image const& image, bool create_mipmaps, bool should_use_srgb)
{
    ZoneScopedN("Texture2D::load(Image)");

    if (image.empty()) {
        // Image not found, defaulting texture to pure white
        fill(Color::White);
        return;
    }

    size = image.get_size();
    colorspace = recover_colorspace(image.get_colorspace(), should_use_srgb);
    data_type = (image.get_data_type() == ImageDataType::FLOAT ? TextureDataType::FLOAT16 : TextureDataType::BYTE);

#if defined(USE_OPENGL_ES)
    if ((size.x & (size.x - 1)) != 0 || (size.y & (size.y - 1)) != 0) {
        Log::vwarning(
            "[Texture] The given image's dimensions ({}x{}) are not powers of two; this can give unexpected results.",
            size.x, size.y
        );
    }
#endif

    int unpack_alignment = 4;

    if (image.get_channel_count() == 1) {
        Renderer::get_parameter(StateParameter::UNPACK_ALIGNMENT, &unpack_alignment);
        Renderer::set_pixel_storage(PixelStorage::UNPACK_ALIGNMENT, 1);
    }

    bind();

    Renderer::send_image_data_2d(
        TextureType::TEXTURE_2D, 0, recover_internal_format(colorspace, data_type), size, recover_format(colorspace),
        (data_type == TextureDataType::BYTE ? PixelDataType::UBYTE : PixelDataType::FLOAT), image.data()
    );

    if (image.get_channel_count() == 1) {
        Renderer::set_pixel_storage(PixelStorage::UNPACK_ALIGNMENT, unpack_alignment);
    }

    set_loaded_parameters(create_mipmaps);
}

void Texture2D::fill(Color const& color)
{
    ZoneScopedN("Texture2D::fill");

    colorspace = TextureColorspace::RGB;
    data_type = TextureDataType::BYTE;

    std::vector<Vector3ub> const values(
        size.x * size.y * 3, Vector3ub(color.r * 255.f, color.g * 255.f, color.b * 255.f)
    );

    bind();
    Renderer::send_image_data_2d(
        TextureType::TEXTURE_2D, 0, TextureInternalFormat::RGB, size, TextureFormat::RGB, PixelDataType::UBYTE,
        values.data()
    );
    unbind();
}

Image Texture2D::recover_image() const
{
    ZoneScopedN("Texture2D::recover_image");

    Image image(
        size, static_cast<ImageColorspace>(colorspace),
        (data_type == TextureDataType::BYTE ? ImageDataType::BYTE : ImageDataType::FLOAT)
    );

    PixelDataType const pixel_data_type =
        (data_type == TextureDataType::BYTE ? PixelDataType::UBYTE : PixelDataType::FLOAT);

#if !defined(USE_OPENGL_ES)
    bind();
    Renderer::recover_texture_data(
        TextureType::TEXTURE_2D, 0, recover_format(colorspace), pixel_data_type, image.data()
    );
    unbind();
#else

    // Recovering an image directly from a texture (glGetTexImage()) is not possible with OpenGL ES; a framebuffer must
    // be used to read the texture from instead See: https://stackoverflow.com/a/53993894/3292304

    Framebuffer const dummy_framebuffer;
    Renderer::bind_framebuffer(dummy_framebuffer.get_index(), FramebufferType::READ_FRAMEBUFFER);

    Renderer::set_framebuffer_texture_2d(
        FramebufferAttachment::COLOR0, index, 0, TextureType::TEXTURE_2D, FramebufferType::READ_FRAMEBUFFER
    );
    Renderer::recover_frame(size, recover_format(colorspace), pixel_data_type, image.data());

    Renderer::unbind_framebuffer(FramebufferType::READ_FRAMEBUFFER);
#endif

    return image;
}

void Texture2D::load() const
{
    ZoneScopedN("Texture2D::load");

    if (colorspace == TextureColorspace::INVALID) {
        return; // No colorspace has been set yet, the texture can't be loaded
    }

    bind();
    Renderer::send_image_data_2d(
        TextureType::TEXTURE_2D, 0, recover_internal_format(colorspace, data_type), size, recover_format(colorspace),
        (data_type == TextureDataType::BYTE ? PixelDataType::UBYTE : PixelDataType::FLOAT), nullptr
    );
    unbind();
}

Texture3D::Texture3D() : Texture(TextureType::TEXTURE_3D) {}

Texture3D::Texture3D(Vector3ui const& size, TextureColorspace colorspace, TextureDataType data_type) :
    Texture3D(colorspace, data_type)
{
    resize(size);
}

Texture3D::Texture3D(Color const& color, Vector3ui size) : Texture3D()
{
    this->size = size;

    fill(color);
}

void Texture3D::resize(Vector3ui const& size)
{
    this->size = size;

    load();
}

void Texture3D::load(std::vector<Image> const& image_slices, bool create_mipmaps, bool should_use_srgb)
{
    ZoneScopedN("Texture3D::load(std::vector<Image>)");

    if (image_slices.empty() || image_slices.front().empty()) {
        // Images not found, defaulting texture to pure white
        fill(Color::White);
        return;
    }

    ImageColorspace const first_image_colorspace = image_slices.front().get_colorspace();
    ImageDataType const first_image_data_type = image_slices.front().get_data_type();

    size.x = image_slices.front().get_width();
    size.y = image_slices.front().get_height();
    size.z = static_cast<uint32_t>(image_slices.size());
    colorspace = recover_colorspace(first_image_colorspace, should_use_srgb);
    data_type = (first_image_data_type == ImageDataType::FLOAT ? TextureDataType::FLOAT16 : TextureDataType::BYTE);

#if defined(USE_OPENGL_ES)
    if ((size.x & (size.x - 1)) != 0 || (size.y & (size.y - 1)) != 0 || (size.z & (size.z - 1)) != 0) {
        Log::vwarning(
            "[Texture] The given image's dimensions ({}x{}x{}) are not powers of two; this can give unexpected "
            "results.",
            size.x, size.y, size.z
        );
    }
#endif

    load();

    TextureFormat const texture_format = recover_format(colorspace);
    PixelDataType const pixel_data_type =
        (data_type == TextureDataType::BYTE ? PixelDataType::UBYTE : PixelDataType::FLOAT);

    bind();

    for (size_t image_index = 0; image_index < image_slices.size(); ++image_index) {
        Image const& image = image_slices[image_index];

        if (image.get_width() != size.x || image.get_height() != size.y ||
            image.get_colorspace() != first_image_colorspace || image.get_data_type() != first_image_data_type)
            throw std::invalid_argument("[Texture3D] The given images have different attributes.");

        Renderer::send_image_sub_data_3d(
            TextureType::TEXTURE_3D, 0, Vector3ui(0, 0, static_cast<uint32_t>(image_index)),
            Vector3ui(size.x, size.y, 1), texture_format, pixel_data_type, image.data()
        );
    }

    set_loaded_parameters(create_mipmaps);
}

void Texture3D::fill(Color const& color)
{
    ZoneScopedN("Texture3D::fill");

    colorspace = TextureColorspace::RGB;
    data_type = TextureDataType::BYTE;

    std::vector<Vector3ub> const values(
        size.x * size.y * size.x * 3, Vector3ub(color.r * 255.f, color.g * 255.f, color.b * 255.f)
    );

    bind();
    Renderer::send_image_data_3d(
        TextureType::TEXTURE_3D, 0, TextureInternalFormat::RGB, size, TextureFormat::RGB, PixelDataType::UBYTE,
        values.data()
    );
    unbind();
}

void Texture3D::load() const
{
    ZoneScopedN("Texture3D::load");

    if (colorspace == TextureColorspace::INVALID) {
        return; // No colorspace has been set yet, the texture can't be loaded
    }

    bind();
    Renderer::send_image_data_3d(
        TextureType::TEXTURE_3D, 0, recover_internal_format(colorspace, data_type), size, recover_format(colorspace),
        (data_type == TextureDataType::BYTE ? PixelDataType::UBYTE : PixelDataType::FLOAT), nullptr
    );
    unbind();
}

}
