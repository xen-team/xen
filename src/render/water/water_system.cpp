#include "water_system.hpp"

#include <scene/scene.hpp>
#include <scene/component.hpp>
#include <scene/components/camera.hpp>
#include <scene/components/water.hpp>
#include <scene/components/transform.hpp>
#include <platform/opengl/framebuffer.hpp>

namespace xen {
void WaterSystem::start()
{
    find_closest_water();
}

void WaterSystem::update()
{
    find_closest_water();
}

// TODO: Should use camera component's position
void WaterSystem::find_closest_water()
{
    // Reset our pointers since it is possible no water exists anymore
    closest_water_component = nullptr;
    float closest_distance_squared = std::numeric_limits<float>::max();

    auto group = scene->get_entity_manager().get_registry().view<TransformComponent, WaterComponent>();
    for (auto entity : group) {
        // Water must have a mesh, otherwise nothing to render water on
        auto [transform_component, water_component] = group.get<TransformComponent, WaterComponent>(entity);

        float const current_distance_squared =
            scene->get_camera().get_position().distance_squared(transform_component.position);

        if (current_distance_squared < closest_distance_squared) {
            closest_distance_squared = current_distance_squared;
            closest_water_component = &water_component;
            closest_water_transform = &transform_component;
        }
    }

    if (closest_water_component) {
        // TODO:
        // Ideally we won't be re-allocating everytime we encounter a different sized reflection/refraction buffer. This
        // NEEDS to be solved if we ever allow multiple water surfaces reflecting and refracting in a single scene Just
        // allocate the biggest and only render to a portion with glViewPort, and make sure when we sample we account
        // for the smaller size as well
        Vector2ui const required_reflection_resolution =
            get_water_reflection_refraction_quality_resolution(closest_water_component->water_reflection_resolution);

        if (closest_water_component->reflection_enabled &&
            (!reflection_fb || reflection_fb->get_width() != required_reflection_resolution.x ||
             reflection_fb->get_height() != required_reflection_resolution.y ||
             (reflection_fb->is_multisampled() != closest_water_component->reflection_msaa))) {
            if (closest_water_component->reflection_msaa) {
                reallocate_reflection_target(reflection_fb, required_reflection_resolution, true);
            }
            else {
                reallocate_reflection_target(reflection_fb, required_reflection_resolution, false);
            }
        }

        Vector2ui const required_refraction_resolution =
            get_water_reflection_refraction_quality_resolution(closest_water_component->water_refraction_resolution);

        if (closest_water_component->reflection_enabled &&
            (!refraction_fb || refraction_fb->get_width() != required_refraction_resolution.x ||
             refraction_fb->get_height() != required_refraction_resolution.y ||
             (refraction_fb->is_multisampled() != closest_water_component->refraction_msaa))) {
            if (closest_water_component->refraction_msaa) {
                reallocate_refraction_target(refraction_fb, required_refraction_resolution, true);
            }
            else {
                reallocate_refraction_target(refraction_fb, required_refraction_resolution, false);
            }
        }

        // If we are using reflection MSAA maybe re-allocate that target if it is required
        if (closest_water_component->reflection_enabled && closest_water_component->reflection_msaa &&
            (!resolve_reflection_fb || resolve_reflection_fb->get_width() != required_reflection_resolution.x ||
             resolve_reflection_fb->get_height() != required_reflection_resolution.y)) {
            reallocate_reflection_target(resolve_reflection_fb, required_reflection_resolution, false);
        }
        if (closest_water_component->refraction_enabled && closest_water_component->refraction_msaa &&
            (!resolve_refraction_fb || resolve_refraction_fb->get_width() != required_refraction_resolution.x ||
             resolve_refraction_fb->get_height() != required_refraction_resolution.y)) {
            reallocate_refraction_target(resolve_refraction_fb, required_refraction_resolution, false);
        }
    }
}

void WaterSystem::reallocate_reflection_target(
    std::unique_ptr<Framebuffer>& framebuffer, Vector2ui const& new_resolution, bool const multisampled
)
{
    framebuffer = std::make_unique<Framebuffer>(new_resolution, multisampled);
    framebuffer->add_color_texture(ColorAttachmentFormat::FloatingPoint16)
        .add_depth_stencil_rbo(DepthStencilAttachmentFormat::NormalizedDepthOnly)
        .create_framebuffer();
}

void WaterSystem::reallocate_refraction_target(
    std::unique_ptr<Framebuffer>& framebuffer, Vector2ui const& new_resolution, bool const multisampled
)
{
    framebuffer = std::make_unique<Framebuffer>(new_resolution, multisampled);
    framebuffer->add_color_texture(ColorAttachmentFormat::FloatingPoint16)
        .add_depth_stencil_texture(DepthStencilAttachmentFormat::NormalizedDepthOnly)
        .create_framebuffer();
}

Vector2f WaterSystem::get_closest_water_reflection_near_far_plane() const
{
    if (!closest_water_component) {
        Log::rt_assert(
            false, "Render::WaterSystem "
                   "Water with Reflection does not exist in current scene - could not get near/far plane"
        );

        return {water_reflection_near_plane_default, water_reflection_far_plane_default};
    }

    return {closest_water_component->reflection_near_plane, closest_water_component->reflection_far_plane};
}

Vector2f WaterSystem::get_closest_water_refraction_near_far_plane() const
{
    if (!closest_water_component) {
        Log::rt_assert(
            false, "Render::WaterSystem "
                   "Water with Refraction does not exist in current scene - could not get near/far plane"
        );

        return {water_refraction_near_plane_default, water_refraction_far_plane_default};
    }

    return {closest_water_component->refraction_near_plane, closest_water_component->refraction_far_plane};
}
}