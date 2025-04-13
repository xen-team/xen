#include "texture.hpp"

#include "platform/opengl/gl_utils.hpp"
#include "platform/opengl/image/image_loader.hpp"
#include "debug/log.hpp"

namespace xen {
constexpr std::array<GLenum, 18> format_table = {GL_R8,
                                                 GL_R16,
                                                 GL_RG8,
                                                 GL_RG16,
                                                 GL_R16F,
                                                 GL_R32F,
                                                 GL_RG16F,
                                                 GL_RG32F,
                                                 GL_RGB,
                                                 GL_RGBA,
                                                 GL_RGB16,
                                                 GL_RGB16F,
                                                 GL_RGBA16,
                                                 GL_RGBA16F,
                                                 GL_RGB32F,
                                                 GL_RGBA32F,
                                                 GL_DEPTH_COMPONENT,
                                                 GL_DEPTH_COMPONENT32F};

constexpr std::array<GLint, 4> wrap_table = {
    GL_CLAMP_TO_EDGE,
    GL_CLAMP_TO_BORDER,
    GL_MIRRORED_REPEAT,
    GL_REPEAT,
};

void Texture::free()
{
    if (id != 0) {
        GL_CALL(glDeleteTextures(1, &id));
        Log::debug("OpenGL::Texture", " freed texture with id: ", id);
    }
    id = 0;
    active_id = 0;
}

Texture::Texture()
{
    GL_CALL(glGenTextures(1, &id));
    Log::debug("OpenGL::Texture", " created texture with id: ", id);
}

Texture::Texture(Texture&& texture) noexcept :
    filepath(std::move(texture.filepath)), width(texture.width), height(texture.height), id(texture.id),
    texture_type(texture.texture_type), format(texture.format), samples(texture.samples)
{

    texture.id = 0;
    texture.active_id = 0;
    texture.width = 0;
    texture.height = 0;
    texture.filepath = "[[deleted]]";
    texture.samples = 0;
}

Texture& Texture::operator=(Texture&& texture) noexcept
{
    free();

    width = texture.width;
    height = texture.height;
    texture_type = texture.texture_type;
    filepath = std::move(texture.filepath);
    samples = texture.samples;
    format = texture.format;
    id = texture.id;

    texture.id = 0;
    texture.active_id = 0;
    texture.width = 0;
    texture.height = 0;
    texture.filepath = "[[deleted]]";
    texture.samples = 0;

    return *this;
}

Texture::~Texture()
{
    free();
}

void Texture::load(std::string_view filepath, TextureFormat format)
{
    constexpr bool flip = true;
    Image image = ImageLoader::load_image(filepath, flip);

    if (image.get_raw_data() == nullptr) {
        Log::error("OpenGL::Texture", " file with name '", filepath, "' was not found or cannot be loaded");
    }

    this->filepath = std::filesystem::proximate(filepath);
    std::ranges::replace(this->filepath, '\\', '/');

    this->format = format;
    this->width = image.get_width();
    this->height = image.get_height();
    this->texture_type = GL_TEXTURE_2D;

    size_t const channels = image.get_channel_count();
    GLenum pixel_format = GL_RGBA;
    switch (channels) {
    case 1:
        pixel_format = GL_RED;
        break;
    case 2:
        pixel_format = GL_RG;
        break;
    case 3:
        pixel_format = GL_RGB;
        break;
    case 4:
        pixel_format = GL_RGBA;
        break;
    default:
        Log::error("OpenGL::Texture", " invalid channel count: ", channels);
        break;
    }

    GL_CALL(glBindTexture(GL_TEXTURE_2D, id));
    GL_CALL(glTexImage2D(
        GL_TEXTURE_2D, 0, format_table[static_cast<int>(this->format)], static_cast<GLsizei>(width),
        static_cast<GLsizei>(height), 0, pixel_format, GL_UNSIGNED_BYTE, image.get_raw_data()
    ));

    if (image.get_raw_data() != nullptr) {
        generate_mipmaps();
    }
    else {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    }
}

Texture::Texture(std::string_view filepath, TextureFormat format) : Texture()
{
    load(filepath, format);
}

void Texture::load(RawDataPointer data, int width, int height, int channels, TextureFormat format)
{
    // this->filepath = MXENGINE_MAKE_INTERNAL_TAG("raw");
    this->width = width;
    this->height = height;
    this->texture_type = GL_TEXTURE_2D;
    this->format = format;

    GLenum data_channels = GL_RGB;
    switch (channels) {
    case 1:
        data_channels = GL_RED;
        break;
    case 2:
        data_channels = GL_RG;
        break;
    case 3:
        data_channels = GL_RGB;
        break;
    case 4:
        data_channels = GL_RGBA;
        break;
    default:
        Log::error("OpenGL::Texture", " invalid channel count: ", channels);
        break;
    }

    GL_CALL(glBindTexture(GL_TEXTURE_2D, id));
    GL_CALL(glTexImage2D(
        GL_TEXTURE_2D, 0, format_table[static_cast<int>(this->format)], static_cast<GLsizei>(width),
        static_cast<GLsizei>(height), 0, data_channels, GL_UNSIGNED_BYTE, data
    ));

    if (data != nullptr) {
        generate_mipmaps();
    }
    else {
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    }
}

void Texture::load(Image const& image, TextureFormat format)
{
    load(
        image.get_raw_data(), static_cast<int>(image.get_width()), static_cast<int>(image.get_height()),
        static_cast<int>(image.get_channel_count()), format
    );
}

void Texture::load_depth(int width, int height, TextureFormat format)
{
    // this->filepath = MXENGINE_MAKE_INTERNAL_TAG("depth");
    this->width = width;
    this->height = height;
    this->texture_type = GL_TEXTURE_2D;
    this->format = format;

    bind();

    GL_CALL(glTexImage2D(
        GL_TEXTURE_2D, 0, format_table[static_cast<int>(this->format)], width, height, 0, GL_DEPTH_COMPONENT,
        GL_UNSIGNED_BYTE, nullptr
    ));

    set_border_color(Vector4f(1.f));

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void Texture::set_max_lod(size_t lod)
{
    bind(0);
    GL_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, (float)lod));
}

void Texture::set_min_lod(size_t lod)
{
    bind(0);
    GL_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, (float)lod));
}

size_t Texture::get_max_texture_lod() const
{
    return static_cast<size_t>(std::log2(std::max(width, height)));
}

Image Texture::get_raw_texture_data()
{
    if (height == 0 || width == 0) {
        return {};
    }

    size_t const pixel_size = get_channel_count() * sizeof(uint8_t);
    size_t const total_byte_size = width * height * pixel_size;
    auto* result = (uint8_t*)std::malloc(total_byte_size);

    GLenum read_format = GL_RGBA;
    switch (get_channel_count()) {
    case 1:
        read_format = GL_RED;
        break;
    case 2:
        read_format = GL_RG;
        break;
    case 3:
        read_format = GL_RGB;
        break;
    case 4:
        read_format = GL_RGBA;
        break;
    default:
        Log::error("OpenGL::Texture", " invalid channel count: ", get_channel_count());
        break;
    }

    bind(0);

    GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL_CALL(glGetTexImage(texture_type, 0, read_format, GL_UNSIGNED_BYTE, (void*)result));

    return {result, width, height, get_channel_count()};
}

void Texture::generate_mipmaps()
{
    bind(0);

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
}

void Texture::set_border_color(Vector4f const& color)
{
    bind(0);

    auto normalized = std::clamp(color, Vector4f(0.f), Vector4f(1.f));

    GL_CALL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &normalized[0]));
}

Vector4f Texture::get_border_color()
{
    Vector4 result = Vector4f(0.f);

    bind(0);

    GL_CALL(glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &result[0]));

    return result;
}

bool Texture::is_multisampled() const
{
    return texture_type == GL_TEXTURE_2D_MULTISAMPLE;
}

bool Texture::is_depth_only() const
{
    return format == TextureFormat::DEPTH || format == TextureFormat::DEPTH32F;
}

size_t Texture::get_sample_count() const
{
    return static_cast<size_t>(samples);
}

size_t Texture::get_pixel_size() const
{
    switch (format) {
        using enum TextureFormat;
    case R:
        return 1;
    case R16:
        return 2;
    case RG:
        return 2;
    case RG16:
        return 4;
    case R16F:
        return 2;
    case R32F:
        return 4;
    case RG16F:
        return 4;
    case RG32F:
        return 8;
    case RGB:
        return 3;
    case RGBA:
        return 4;
    case RGB16:
        return 6;
    case RGB16F:
        return 6;
    case RGBA16:
        return 8;
    case RGBA16F:
        return 8;
    case RGB32F:
        return 12;
    case RGBA32F:
        return 16;
    case DEPTH:
        return 1;
    case DEPTH32F:
        return 4;
    default:
        return 0;
    }
}

TextureWrap Texture::get_wrap_type()
{
    GLint result = 0;
    bind(0);

    GL_CALL(glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &result));

    for (size_t i = 0; i < wrap_table.size(); i++) {
        if (wrap_table[i] == result) {
            return TextureWrap(i);
        }
    }

    return TextureWrap::CLAMP_TO_EDGE;
}

void Texture::set_wrap_type(TextureWrap wrap_type)
{
    bind(0);

    GL_CALL(glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &wrap_table[static_cast<int>(wrap_type)]));
    GL_CALL(glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &wrap_table[static_cast<int>(wrap_type)]));
}

void Texture::bind()
{
    GL_CALL(glActiveTexture(GL_TEXTURE0 + active_id));
    GL_CALL(glBindTexture(texture_type, id));
}

void Texture::unbind()
{
    GL_CALL(glActiveTexture(GL_TEXTURE0 + active_id));
    GL_CALL(glBindTexture(texture_type, 0));
}

void Texture::bind(TextureHandle id)
{
    active_id = id;
    bind();
}

// void Texture::SetInternalEngineTag(MxString const& tag)
// {
//     this->filepath = tag;
// }

// bool Texture::IsInternalEngineResource() const
// {
//     return this->filepath.find(MXENGINE_INTERNAL_TAG_SYMBOL) == 0;
// }

size_t Texture::get_channel_count() const
{
    switch (format) {
        using enum TextureFormat;
    case R:
        return 1;
    case R16:
        return 1;
    case RG:
        return 2;
    case RG16:
        return 2;
    case R16F:
        return 1;
    case R32F:
        return 1;
    case RG16F:
        return 2;
    case RG32F:
        return 2;
    case RGB:
        return 3;
    case RGBA:
        return 4;
    case RGB16:
        return 3;
    case RGB16F:
        return 3;
    case RGBA16:
        return 4;
    case RGBA16F:
        return 4;
    case RGB32F:
        return 3;
    case RGBA32F:
        return 4;
    case DEPTH:
        return 1;
    case DEPTH32F:
        return 1;
    default:
        return 0;
    }
}
}