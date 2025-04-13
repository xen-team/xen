#include "cubemap.hpp"
#include <GL/glext.h>

namespace xen {
Cubemap::Cubemap(CubemapSettings& settings) : settings(settings) {}

Cubemap::~Cubemap()
{
    GL_CALL(glDeleteTextures(1, &id));
    Log::debug(
        "Render::Cubemap "
        "deleted cubemap with id: ",
        id
    );
    id = 0;
}

void Cubemap::apply_cubemap_settings()
{
    // Texture wrapping
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, settings.texture_wrap_s_mode));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, settings.texture_wrap_t_mode));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, settings.texture_wrap_r_mode));

    // Texture filtering
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, settings.texture_magnification_filter_mode));
    GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, settings.texture_minification_filter_mode));

    // Mipmapping
    if (settings.has_mips) {
        GL_CALL(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
        GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_LOD_BIAS, settings.mip_bias));
    }

    // Anisotropic filtering (TODO: Move the anistropyAmount calculation to Defs.h to avoid querying the OpenGL driver
    // everytime)
    float max_anisotropy;
    GL_CALL(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy));

    float anistropy_amount = std::min(max_anisotropy, settings.texture_anisotropy_level);
    GL_CALL(glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, anistropy_amount));
}

void Cubemap::generate_cubemap(Vector2ui const& size, GLenum const data_format)
{
    Log::debug(
        "Render::Cubemap "
        "created cubemap with id: ",
        id
    );

    GL_CALL(glGenTextures(1, &id));

    this->size = size;

    // If GL_NONE is specified, set the texture format to the data format
    if (settings.texture_format == GL_NONE) {
        settings.texture_format = data_format;
    }
    // Check if the texture is SRGB, if so change the texture format
    if (settings.is_srgb) {
        switch (data_format) {
        case GL_RGB:
            settings.texture_format = GL_SRGB;
            break;
        case GL_RGBA:
            settings.texture_format = GL_SRGB_ALPHA;
            break;
        default:
            Log::warning("Render::Cubemap "
                         "generate_cubemap_face() incorrect texture format type");

            settings.texture_format = GL_SRGB;
            break;
        }
    }
}

void Cubemap::generate_cubemap_face(
    GLenum const face, Vector2ui const& size, GLenum const data_format, uint8_t const* data
)
{
    // Generate cubemap if this is the first face being generated
    if (id == 0) {
        generate_cubemap(size, data_format);
    }

    bind();

    GL_CALL(glTexImage2D(
        face, 0, settings.texture_format, this->size.x, this->size.y, 0, data_format, GL_UNSIGNED_BYTE, data
    ));

    ++faces_generated;

    if (faces_generated >= 6) {
        apply_cubemap_settings();
    }

    unbind();
}

void Cubemap::generate_cubemap_faces(
    uint const count, GLenum const face, Vector2ui const& size, GLenum const data_format, uint8_t const* data
)
{
    // Generate cubemap if this is the first face being generated
    if (id == 0) {
        generate_cubemap(size, data_format);
    }

    bind();

    for (uint i = 0; i < count; i++, faces_generated++) {
        GL_CALL(glTexImage2D(
            face + i, 0, settings.texture_format, this->size.x, this->size.y, 0, data_format, GL_UNSIGNED_BYTE, data
        ));
    }

    if (faces_generated >= 6) {
        apply_cubemap_settings();
    }

    unbind();
}

void Cubemap::bind(int const unit)
{
    GL_CALL(glActiveTexture(GL_TEXTURE0 + unit));
    GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
}

void Cubemap::unbind()
{
    GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}
}