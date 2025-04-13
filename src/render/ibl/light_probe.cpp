#include "light_probe.hpp"

#include <render/shader.hpp>
#include <render/texture/cubemap.hpp>

namespace xen {
LightProbe::LightProbe(Vector3f const& position, Vector2f const& resolution) :
    position(position), resolution(resolution)
{
}

void LightProbe::generate()
{
    // Generate the HDR light probe and set the generated flag
    CubemapSettings settings;
    settings.texture_format = GL_RGBA16F;
    irradiance_map = std::make_unique<Cubemap>(settings);

    irradiance_map->generate_cubemap_faces(6, GL_TEXTURE_CUBE_MAP_POSITIVE_X, Vector2ui(resolution), GL_RGB, nullptr);
    generated = true;
}

void LightProbe::bind(Shader& shader)
{
    irradiance_map->bind(3);
    shader.set_uniform("irradianceMap", 3);
}
}