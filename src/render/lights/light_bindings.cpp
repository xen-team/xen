#include "light_bindings.hpp"

#include <scene/components/light.hpp>
#include <scene/components/transform.hpp>
#include <render/shader.hpp>

namespace xen {
void LightBindings::bind_directional_light(
    TransformComponent const& transform, LightComponent const& light, Shader& shader, uint const current_light_index
)
{
    // , "Exceeded Directional Light Count"
    assert(current_light_index < max_dir_lights);

    shader.set_uniform("dirLights[" + std::to_string(current_light_index) + "].direction", transform.forward());
    shader.set_uniform("dirLights[" + std::to_string(current_light_index) + "].intensity", light.intensity);
    shader.set_uniform(
        "dirLights[" + std::to_string(current_light_index) + "].lightColour", static_cast<Vector3f>(light.color)
    );
}

void LightBindings::bind_point_light(
    TransformComponent const& transform, LightComponent const& light, Shader& shader, uint const current_light_index
)
{
    // , "Exceeded Point Light Count"
    assert(current_light_index < max_point_lights);

    shader.set_uniform("pointLights[" + std::to_string(current_light_index) + "].position", transform.position);
    shader.set_uniform("pointLights[" + std::to_string(current_light_index) + "].intensity", light.intensity);
    shader.set_uniform(
        "pointLights[" + std::to_string(current_light_index) + "].lightColour", static_cast<Vector3f>(light.color)
    );
    shader.set_uniform(
        "pointLights[" + std::to_string(current_light_index) + "].attenuationRadius", light.attenuation_range
    );
}

void LightBindings::bind_spot_light(
    TransformComponent const& transform, LightComponent const& light, Shader& shader, uint const current_light_index
)
{
    // , "Exceeded Spot Light Count"
    assert(current_light_index < max_spot_lights);

    shader.set_uniform("spotLights[" + std::to_string(current_light_index) + "].position", transform.position);
    shader.set_uniform("spotLights[" + std::to_string(current_light_index) + "].direction", transform.forward());
    shader.set_uniform("spotLights[" + std::to_string(current_light_index) + "].intensity", light.intensity);
    shader.set_uniform(
        "spotLights[" + std::to_string(current_light_index) + "].lightColour", static_cast<Vector3f>(light.color)
    );
    shader.set_uniform(
        "spotLights[" + std::to_string(current_light_index) + "].attenuationRadius", light.attenuation_range
    );
    shader.set_uniform("spotLights[" + std::to_string(current_light_index) + "].cutOff", light.inner_cut_off);
    shader.set_uniform("spotLights[" + std::to_string(current_light_index) + "].outerCutOff", light.outer_cut_off);
}
}