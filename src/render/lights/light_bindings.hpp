#pragma once

namespace xen {
class Shader;
struct TransformComponent;
struct LightComponent;

class LightBindings {
public:
    static constexpr int max_dir_lights = 3;
    static constexpr int max_point_lights = 6;
    static constexpr int max_spot_lights = 6;

public:
    static void bind_directional_light(
        TransformComponent const& transform, LightComponent const& light, Shader& shader, uint current_light_index
    );
    static void

    bind_point_light(
        TransformComponent const& transform, LightComponent const& light, Shader& shader, uint current_light_index
    );

    static void bind_spot_light(
        TransformComponent const& transform, LightComponent const& light, Shader& shader, uint current_light_index
    );
};
}