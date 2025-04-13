#pragma once

#include <scene/component.hpp>
#include <render/water/water_system.hpp>

namespace xen {
class Texture;

struct WaterComponent : public Component {
    inline static bool const registered = register_component<WaterComponent>();

    Vector3f water_albedo = Vector3f(0.102f, 0.431f, 0.902f);
    float albedo_power = 0.05f;

    bool reflection_enabled = true;
    WaterReflectionRefractionQuality water_reflection_resolution = WaterReflectionRefractionQuality::High;

    bool refraction_enabled = true;
    WaterReflectionRefractionQuality water_refraction_resolution = WaterReflectionRefractionQuality::High;

    bool reflection_msaa = true;
    bool refraction_msaa = false;
    bool clear_water = false;
    bool enable_shine = true;

    Vector2f water_tiling = Vector2f(6.0f, 6.0f);
    float wave_speed = 0.05f;
    float wave_strength = 0.02f;

    float shine_damper = 300.0f;
    float normal_smoothing = 3.0f;
    float depth_dampening = 0.05f;

    float reflection_plane_bias = 1.5f;
    float refraction_plane_bias = 0.5f;

    float reflection_near_plane = water_reflection_near_plane_default;
    float reflection_far_plane = water_reflection_far_plane_default;

    float refraction_near_plane = water_refraction_near_plane_default;
    float refraction_far_plane = water_refraction_far_plane_default;

    float move_timer = 0.0f; // Should not be set or used by the user. Just used for water rendering, that is why this
                             // isn't viewable/modifiable in the inspector panel

    std::shared_ptr<Texture> water_distortion_texture;
    std::shared_ptr<Texture> water_normal_map;

    template <class Archive>
    void save(Archive& ar) const
    {
        // ar(water_albedo, albedo_power, reflection_enabled, water_reflection_resolution, refraction_enabled,
        //    water_refraction_resolution, reflection_msaa, refraction_msaa, clear_water, enable_shine, water_tiling,
        //    wave_speed, wave_strength, shine_damper, normal_smoothing, depth_dampening, reflection_plane_bias,
        //    refraction_plane_bias, reflection_near_plane, reflection_far_plane, refraction_near_plane,
        //    refraction_far_plane, move_timer, water_distortion_texture, water_normal_map);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        // ar(water_albedo, albedo_power, reflection_enabled, water_reflection_resolution, refraction_enabled,
        //    water_refraction_resolution, reflection_msaa, refraction_msaa, clear_water, enable_shine, water_tiling,
        //    wave_speed, wave_strength, shine_damper, normal_smoothing, depth_dampening, reflection_plane_bias,
        //    refraction_plane_bias, reflection_near_plane, reflection_far_plane, refraction_near_plane,
        //    refraction_far_plane, move_timer, water_distortion_texture, water_normal_map);
    }
};
}
