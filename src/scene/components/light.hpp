#pragma once

#include <scene/component.hpp>
#include <render/lights/light_system.hpp>
#include <utils/color.hpp>

namespace xen {
struct LightComponent : public Component {
    inline static bool const registered = register_component<LightComponent>();

    LightType type = LightType::Point;

    float intensity = 1.0f;
    Color color = Color(1.f, 1.f, 1.f);
    float attenuation_range = 10.0f; // Used for spot and point lights only

    // Used for spot lights and stored as [0, 1] as cos(angle)
    float inner_cut_off = std::cos(Math::deg_to_rad(25.0f));
    float outer_cut_off = std::cos(Math::deg_to_rad(27.5f));

    bool is_static = false; // Should be true if the light will never have any of it's properties changed (then we can
                            // use it for global illumination since it is static)

    bool cast_shadows = false;
    float shadow_bias = shadowmap_bias_default;
    ShadowQuality shadow_resolution = ShadowQuality::Medium;
    float shadow_near_plane = shadowmap_near_plane_default;
    float shadow_far_plane = shadowmap_far_plane_default;

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(type, intensity, color, attenuation_range, inner_cut_off, outer_cut_off, is_static, cast_shadows,
           shadow_bias, shadow_resolution, shadow_near_plane, shadow_far_plane);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(type, intensity, color, attenuation_range, inner_cut_off, outer_cut_off, is_static, cast_shadows,
           shadow_bias, shadow_resolution, shadow_near_plane, shadow_far_plane);
    }
};
}
