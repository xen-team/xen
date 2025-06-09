#include "material.hpp"

#include <render/renderer.hpp>

#include <tracy/Tracy.hpp>

namespace {
constexpr std::string_view vert_shader_source = {
#include "common.vert.embed"
};

constexpr std::string_view cook_torrance_shader_source = {
#include "cook-torrance.frag.embed"
};

constexpr std::string_view blinn_phong_shader_source = {
#include "blinn-phong.frag.embed"
};

constexpr std::string_view single_texture_2d_shader_source = {
#include "single_texture_2d.frag.embed"
};

constexpr std::string_view single_texture_3d_shader_source = {
#include "single_texture_3d.frag.embed"
};
}

namespace xen {
void Material::load_type(MaterialType type)
{
    ZoneScopedN("Material::load_type");

    switch (type) {
    case MaterialType::COOK_TORRANCE:
        program.set_shaders(
            VertexShader::load_from_source(vert_shader_source),
            FragmentShader::load_from_source(cook_torrance_shader_source)
        );

        if (!program.has_attribute(MaterialAttribute::BaseColor)) {
            program.set_attribute(Vector3f(1.f), MaterialAttribute::BaseColor);
        }
        if (!program.has_attribute(MaterialAttribute::Emissive)) {
            program.set_attribute(Vector3f(0.f), MaterialAttribute::Emissive);
        }
        if (!program.has_attribute(MaterialAttribute::Metallic)) {
            program.set_attribute(0.f, MaterialAttribute::Metallic);
        }
        if (!program.has_attribute(MaterialAttribute::Roughness)) {
            program.set_attribute(0.f, MaterialAttribute::Roughness);
        }
        if (!program.has_attribute(MaterialAttribute::Sheen)) {
            program.set_attribute(Vector4f(0.f), MaterialAttribute::Sheen);
        }

        if (!program.has_texture(MaterialTexture::BaseColor)) {
            program.set_texture(Texture2D::create(Color::White), MaterialTexture::BaseColor);
        }
        if (!program.has_texture(MaterialTexture::Emissive)) {
            program.set_texture(Texture2D::create(Color::White), MaterialTexture::Emissive);
        }
        if (!program.has_texture(MaterialTexture::Normal)) {
            program.set_texture(
                Texture2D::create(Color::Aqua), MaterialTexture::Normal
            ); // Representing a [ 0; 0; 1 ] vector
        }
        if (!program.has_texture(MaterialTexture::Metallic)) {
            program.set_texture(Texture2D::create(Color::Red), MaterialTexture::Metallic);
        }
        if (!program.has_texture(MaterialTexture::Roughness)) {
            program.set_texture(Texture2D::create(Color::Red), MaterialTexture::Roughness);
        }
        if (!program.has_texture(MaterialTexture::Ambient)) {
            program.set_texture(Texture2D::create(Color::Red), MaterialTexture::Ambient);
        }
        if (!program.has_texture(MaterialTexture::Sheen)) {
            program.set_texture(
                Texture2D::create(Color::White), MaterialTexture::Sheen
            ); // TODO: should be an RGBA texture with an alpha of 1
        }

        break;

    case MaterialType::BLINN_PHONG:
        program.set_shaders(
            VertexShader::load_from_source(vert_shader_source),
            FragmentShader::load_from_source(blinn_phong_shader_source)
        );

        if (!program.has_attribute(MaterialAttribute::BaseColor)) {
            program.set_attribute(Vector3f(1.f), MaterialAttribute::BaseColor);
        }
        if (!program.has_attribute(MaterialAttribute::Emissive)) {
            program.set_attribute(Vector3f(0.f), MaterialAttribute::Emissive);
        }
        if (!program.has_attribute(MaterialAttribute::Ambient)) {
            program.set_attribute(Vector3f(1.f), MaterialAttribute::Ambient);
        }
        if (!program.has_attribute(MaterialAttribute::Specular)) {
            program.set_attribute(Vector3f(1.f), MaterialAttribute::Specular);
        }
        if (!program.has_attribute(MaterialAttribute::Opacity)) {
            program.set_attribute(1.f, MaterialAttribute::Opacity);
        }

        if (!program.has_texture(MaterialTexture::BaseColor)) {
            program.set_texture(Texture2D::create(Color::White), MaterialTexture::BaseColor);
        }
        if (!program.has_texture(MaterialTexture::Emissive)) {
            program.set_texture(Texture2D::create(Color::White), MaterialTexture::Emissive);
        }
        if (!program.has_texture(MaterialTexture::Ambient)) {
            program.set_texture(Texture2D::create(Color::White), MaterialTexture::Ambient);
        }
        if (!program.has_texture(MaterialTexture::Specular)) {
            program.set_texture(Texture2D::create(Color::White), MaterialTexture::Specular);
        }
        if (!program.has_texture(MaterialTexture::Opacity)) {
            Texture2DPtr opacity_map = Texture2D::create(Color::White);
            opacity_map->set_filter(TextureFilter::NEAREST, TextureFilter::NEAREST, TextureFilter::NEAREST);
            program.set_texture(std::move(opacity_map), MaterialTexture::Opacity);
        }
        if (!program.has_texture(MaterialTexture::Bump)) {
            program.set_texture(Texture2D::create(Color::White), MaterialTexture::Bump);
        }

        break;

    case MaterialType::SINGLE_TEXTURE_2D:
        program.set_shaders(
            VertexShader::load_from_source(vert_shader_source),
            FragmentShader::load_from_source(single_texture_2d_shader_source)
        );

        if (!program.has_texture(MaterialTexture::BaseColor)) {
            program.set_texture(Texture2D::create(Color::White), MaterialTexture::BaseColor);
        }

        break;

    case MaterialType::SINGLE_TEXTURE_3D:
        program.set_shaders(
            VertexShader::load_from_source(vert_shader_source),
            FragmentShader::load_from_source(single_texture_3d_shader_source)
        );

        if (!program.has_texture(MaterialTexture::BaseColor)) {
            program.set_texture(Texture3D::create(Color::White), MaterialTexture::BaseColor);
        }

        break;

    default:
        throw std::invalid_argument("Error: Unsupported material type");
    }
}
}
