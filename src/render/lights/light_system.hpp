#pragma once

#include <platform/opengl/framebuffer.hpp>
#include <render/texture/cubemap.hpp>
#include <engine/module.hpp>
#include <scene/system.hpp>

namespace xen {
struct LightComponent;
struct TransformComponent;
class Scene;
class Shader;

enum class LightType : uint8_t { Directional, Point, Spot, COUNT };

enum class ShadowQuality : uint8_t {
    Low,      // 256 x 256
    Medium,   // 512 x 512
    High,     // 1024 x 1024
    Ultra,    // 2048 x 2048
    Nightmare // 4096 x 4096
};

class XEN_API LightSystem : public System {
private:
    // Directional Light Shadows (keeps track of closest one so passes can use these framebuffers for the shadows)
    LightComponent* closest_directional_light_shadow_caster = nullptr;
    TransformComponent* closest_directional_light_shadow_caster_transform = nullptr;
    int closest_directional_light_index = 0;
    std::unique_ptr<Framebuffer> directional_light_shadow_framebuffer;

    // Spot Light Shadows (keeps track of closest one so passes can use these framebuffers for the shadows)
    LightComponent* closest_spot_light_shadow_caster = nullptr;
    TransformComponent* closest_spot_light_shadow_caster_transform = nullptr;
    int closest_spot_light_index = 0;
    std::unique_ptr<Framebuffer> spot_light_shadow_framebuffer;

    // Point Light Shadows (keeps track of closest one so passes can use these framebuffers for the shadows)
    LightComponent* closest_point_light_shadow_caster = nullptr;
    TransformComponent* closest_point_light_shadow_caster_tranform = nullptr;
    int closest_point_light_index = 0;
    std::unique_ptr<Cubemap> point_light_shadow_cubemap;

public:
    void start() override;
    void update() override;

    void bind_lightning_uniforms(Shader& shader);
    void bind_static_lighting_uniforms(Shader& shader);

    constexpr static Vector2ui get_shadow_quality_resolution(ShadowQuality const quality)
    {
        switch (quality) {
        case ShadowQuality::Low:
            return {256, 256};
            break;
        case ShadowQuality::Medium:
            return {512, 512};
            break;
        case ShadowQuality::High:
            return {1024, 1024};
            break;
        case ShadowQuality::Ultra:
            return {2048, 2048};
            break;
        case ShadowQuality::Nightmare:
            return {4096, 4096};
            break;
        }
    }

    // getters for directional light shadow caster
    [[nodiscard]] bool has_directional_light_shadow_caster() const
    {
        return closest_directional_light_shadow_caster != nullptr;
    }
    [[nodiscard]] Framebuffer* get_directional_light_shadow_framebuffer() const
    {
        return directional_light_shadow_framebuffer.get();
    }
    [[nodiscard]] Vector3f get_directional_light_shadow_caster_light_dir() const;
    [[nodiscard]] Vector2f get_directional_light_shadow_caster_near_far_plane() const;
    [[nodiscard]] float get_directional_light_shadow_caster_bias() const;
    [[nodiscard]] int get_directional_light_shadow_caster_index() const;

    // getters for spot light shadow caster
    [[nodiscard]] bool has_spot_light_shadow_caster() const { return closest_spot_light_shadow_caster != nullptr; }
    [[nodiscard]] Framebuffer* get_spot_light_shadow_caster_framebuffer() const
    {
        return spot_light_shadow_framebuffer.get();
    }
    [[nodiscard]] Vector3f get_spot_light_shadow_caster_light_dir() const;
    [[nodiscard]] Vector3f get_spot_light_shadow_caster_light_position() const;
    [[nodiscard]] float
    get_spot_light_shadow_caster_outer_cut_off_angle() const; // Returns outer cutoff angle in radians
    [[nodiscard]] float get_spot_light_shadow_caster_attenuation_range() const;
    [[nodiscard]] Vector2f get_spot_light_shadow_caster_near_far_plane() const;
    [[nodiscard]] float get_spot_light_shadow_caster_bias() const;
    [[nodiscard]] int get_spot_light_shadow_caster_index() const;

    // getters for point light shadow caster
    [[nodiscard]] bool has_pointlight_shadow_caster() const { return closest_point_light_shadow_caster != nullptr; }
    [[nodiscard]] Cubemap* get_point_light_shadow_caster_cubemap() const { return point_light_shadow_cubemap.get(); }
    [[nodiscard]] Vector3f get_point_light_shadow_caster_light_position() const;
    [[nodiscard]] Vector2f get_point_light_shadow_caster_near_far_plane() const;
    [[nodiscard]] float get_point_light_shadow_caster_bias() const;
    [[nodiscard]] int get_point_light_shadow_caster_index() const;

private:
    void find_closest_directional_light_shadow_caster();
    void find_closest_spot_light_shadow_caster();
    void find_closest_point_light_shadow_caster();
    void bind_lights(Shader& shader, bool bind_only_static);
    void reallocate_depth_target(std::unique_ptr<Framebuffer>& framebuffer, Vector2ui const& new_resolution);
    void reallocate_depth_cubemap(std::unique_ptr<Cubemap>& cubemap, Vector2ui const& new_resolution);
};
}