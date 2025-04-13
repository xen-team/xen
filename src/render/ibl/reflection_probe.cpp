#include "reflection_probe.hpp"

#include <render/shader.hpp>
#include <render/texture/cubemap.hpp>

namespace xen {
std::unique_ptr<Texture> ReflectionProbe::brdf_lut = nullptr;

ReflectionProbe::ReflectionProbe(Vector3f const& position, Vector2f const& resolution) :
    position(position), resolution(resolution)
{
}

void ReflectionProbe::generate()
{
    // Generate the HDR reflection probe and set the generated flag
    CubemapSettings settings;
    settings.texture_format = GL_RGBA16F;
    settings.texture_minification_filter_mode = GL_LINEAR_MIPMAP_LINEAR;
    settings.has_mips = true;

    prefilter_map = std::make_unique<Cubemap>(settings);
    prefilter_map->generate_cubemap_faces(6, GL_TEXTURE_CUBE_MAP_POSITIVE_X, Vector2ui(resolution), GL_RGB, nullptr);

    generated = true;
}

void ReflectionProbe::bind(Shader& shader)
{
    shader.set_uniform("reflectionProbeMipCount", reflection_probe_min_count);

    prefilter_map->bind(4);
    shader.set_uniform("prefilterMap", 4);

    brdf_lut->bind(5);
    shader.set_uniform("brdfLUT", 5);
}
}