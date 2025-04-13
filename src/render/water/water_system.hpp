#pragma once

#include <platform/opengl/framebuffer.hpp>
#include <engine/module.hpp>
#include <scene/system.hpp>

namespace xen {
class Scene;
struct WaterComponent;
struct TransformComponent;

enum class WaterReflectionRefractionQuality : uint8_t {
    Low,      // 128 x 128
    Medium,   // 256 x 256
    High,     // 512 x 512
    Ultra,    // 1024 x 1024
    Nightmare // 2048 x 2048
};

// Current imp is bad...
class WaterSystem : public System {
private:
    // Currently only supports one reflection/refraction water surface at a time, so keep track of the closest so it can
    // reflect and refract it so the water pass can then use these at rendering time
    WaterComponent* closest_water_component;
    TransformComponent* closest_water_transform;

    std::unique_ptr<Framebuffer> reflection_fb;
    std::unique_ptr<Framebuffer> refraction_fb;

    std::unique_ptr<Framebuffer> resolve_reflection_fb;
    std::unique_ptr<Framebuffer> resolve_refraction_fb; // Only used for MSAA

public:
    void start() override;
    void update() override;

    constexpr static Vector2ui
    get_water_reflection_refraction_quality_resolution(WaterReflectionRefractionQuality const quality)
    {
        switch (quality) {
            using enum WaterReflectionRefractionQuality;

        case Low:
            return {128, 128};
            break;
        case Medium:
            return {256, 256};
            break;
        case High:
            return {512, 512};
            break;
        case Ultra:
            return {1024, 1024};
            break;
        case Nightmare:
            return {2048, 2048};
            break;
        }
    }

    // Getters for water using reflection/refraction (right now the closest water instance is the one that gets
    // reflection/refraction)
    [[nodiscard]] bool has_water_with_refraction_reflection() const
    {
        return reflection_fb != nullptr && refraction_fb != nullptr;
    }

    Framebuffer* get_water_reflection_framebuffer() { return reflection_fb.get(); }

    Framebuffer* get_water_refraction_framebuffer() { return refraction_fb.get(); }

    Framebuffer* get_water_refraction_resolve_framebuffer()
    {
        return resolve_reflection_fb.get();
    } // MSAA resolve framebuffer for refraction

    Framebuffer* get_water_reflection_resolve_framebuffer()
    {
        return resolve_refraction_fb.get();
    } // MSAA resolve framebuffer for reflection

    [[nodiscard]] WaterComponent const* get_closest_water_component() { return closest_water_component; }

    [[nodiscard]] Vector2f get_closest_water_reflection_near_far_plane() const;

    [[nodiscard]] Vector2f get_closest_water_refraction_near_far_plane() const;

private:
    void find_closest_water();

    void reallocate_reflection_target(
        std::unique_ptr<Framebuffer>& framebuffer, Vector2ui const& new_resolution, bool multisampled
    );

    void reallocate_refraction_target(
        std::unique_ptr<Framebuffer>& framebuffer, Vector2ui const& new_resolution, bool multisampled
    );
};
}