#include "light_system.hpp"

#include <platform/opengl/framebuffer.hpp>

#include <render/lights/light_bindings.hpp>
#include <render/shader.hpp>
#include <render/texture/cubemap.hpp>

#include <scene/components/light.hpp>
#include <scene/components/transform.hpp>
#include <scene/scenes.hpp>

namespace xen {
void LightSystem::start()
{
    find_closest_directional_light_shadow_caster();
    find_closest_spot_light_shadow_caster();
    find_closest_point_light_shadow_caster();

    // Default framebuffers if a shadow isn't found. Hopefully save an allocation when we find one
    if (!directional_light_shadow_framebuffer) {
        directional_light_shadow_framebuffer = std::make_unique<Framebuffer>(
            Vector2f{shadowmap_resolution_x_default, shadowmap_resolution_y_default}, false
        );

        directional_light_shadow_framebuffer
            ->add_depth_stencil_texture(DepthStencilAttachmentFormat::NormalizedDepthOnly, true)
            .create_framebuffer();
    }

    if (!spot_light_shadow_framebuffer) {
        spot_light_shadow_framebuffer = std::make_unique<Framebuffer>(
            Vector2f{shadowmap_resolution_x_default, shadowmap_resolution_y_default}, false
        );

        spot_light_shadow_framebuffer
            ->add_depth_stencil_texture(DepthStencilAttachmentFormat::NormalizedDepthOnly, true)
            .create_framebuffer();
    }
    if (!point_light_shadow_cubemap) {
        reallocate_depth_cubemap(
            point_light_shadow_cubemap, Vector2ui(shadowmap_resolution_x_default, shadowmap_resolution_y_default)
        );
    }
}

void LightSystem::update()
{
    find_closest_directional_light_shadow_caster();
    find_closest_spot_light_shadow_caster();
    find_closest_point_light_shadow_caster();
}

// TODO: Should use camera component's position
void LightSystem::find_closest_directional_light_shadow_caster()
{
    float closest_distance_squared = std::numeric_limits<float>::max();
    int current_directional_light_index = -1;

    // Prioritize the closest directional light to the camera as our directional shadow caster
    auto group = Scenes::get()
                     ->get_scene()
                     ->get_entity_manager()
                     .get_registry()
                     .group<LightComponent>(entt::get<TransformComponent>);
    for (auto entity : group) {
        auto [transform, light] = group.get<TransformComponent, LightComponent>(entity);

        if (light.type != LightType::Directional) {
            continue;
        }

        current_directional_light_index++;

        if (!light.cast_shadows) {
            continue;
        }

        float const current_distance_squared =
            Math::distance_squared(Scenes::get()->get_scene()->get_camera().get_position(), transform.position);

        if (current_distance_squared < closest_distance_squared) {
            closest_distance_squared = current_distance_squared;
            closest_directional_light_shadow_caster = &light;
            closest_directional_light_shadow_caster_transform = &transform;
            closest_directional_light_index = current_directional_light_index;
        }
    }

    if (closest_directional_light_shadow_caster) {
        // TODO:
        // Ideally we won't be re-allocating everytime we encounter a different sized shadow map. This NEEDS to be
        // solved if we ever allow multiple directional light shadow casters Just allocate the biggest and only render
        // to a portion with glViewPort, and make sure when we sample the shadowmap we account for the smaller size as
        // well
        Vector2ui const required_shadow_resolution =
            get_shadow_quality_resolution(closest_directional_light_shadow_caster->shadow_resolution);

        if (!directional_light_shadow_framebuffer ||
            directional_light_shadow_framebuffer->get_width() != required_shadow_resolution.x ||
            directional_light_shadow_framebuffer->get_height() != required_shadow_resolution.y) {
            reallocate_depth_target(directional_light_shadow_framebuffer, required_shadow_resolution);
        }
    }
}

// TODO: Should use camera component's position
void LightSystem::find_closest_spot_light_shadow_caster()
{
    float closest_distance_squared = std::numeric_limits<float>::max();
    int current_spot_light_index = -1;

    // Prioritize the closest spot light to the camera as our spotlight shadow caster
    auto group = Scenes::get()
                     ->get_scene()
                     ->get_entity_manager()
                     .get_registry()
                     .group<LightComponent>(entt::get<TransformComponent>);
    for (auto entity : group) {
        auto [transform, light] = group.get<TransformComponent, LightComponent>(entity);

        if (light.type != LightType::Spot) {
            continue;
        }

        current_spot_light_index++;

        if (!light.cast_shadows) {
            continue;
        }

        float const current_distance_squared =
            Math::distance_squared(Scenes::get()->get_scene()->get_camera().get_position(), transform.position);

        if (current_distance_squared < closest_distance_squared) {
            closest_distance_squared = current_distance_squared;
            closest_spot_light_shadow_caster = &light;
            closest_spot_light_shadow_caster_transform = &transform;
            closest_spot_light_index = current_spot_light_index;
        }
    }

    if (closest_spot_light_shadow_caster) {
        // TODO:
        // Ideally we won't be re-allocating everytime we encounter a different sized shadow map. This NEEDS to be
        // solved if we ever allow multiple spot light shadow casters Just allocate the biggest and only render to a
        // portion with glViewPort, and make sure when we sample the shadowmap we account for the smaller size as well
        Vector2ui const required_shadow_resolution =
            get_shadow_quality_resolution(closest_spot_light_shadow_caster->shadow_resolution);

        if (!spot_light_shadow_framebuffer ||
            spot_light_shadow_framebuffer->get_width() != required_shadow_resolution.x ||
            spot_light_shadow_framebuffer->get_height() != required_shadow_resolution.y) {
            reallocate_depth_target(spot_light_shadow_framebuffer, required_shadow_resolution);
        }
    }
}

// TODO: Should use camera component's position
void LightSystem::find_closest_point_light_shadow_caster()
{
    float closest_distance_squared = std::numeric_limits<float>::max();
    int current_point_light_index = -1;

    // Prioritize the closest point light to the camera as our pointlight shadow caster
    auto group = Scenes::get()
                     ->get_scene()
                     ->get_entity_manager()
                     .get_registry()
                     .group<LightComponent>(entt::get<TransformComponent>);
    for (auto entity : group) {
        auto [transform, light] = group.get<TransformComponent, LightComponent>(entity);

        if (light.type != LightType::Point) {
            continue;
        }

        current_point_light_index++;

        if (!light.cast_shadows) {
            continue;
        }

        float const current_distance_squared =
            Math::distance_squared(Scenes::get()->get_scene()->get_camera().get_position(), transform.position);

        if (current_distance_squared < closest_distance_squared) {
            closest_distance_squared = current_distance_squared;
            closest_point_light_shadow_caster = &light;
            closest_point_light_shadow_caster_tranform = &transform;
            closest_point_light_index = current_point_light_index;
        }
    }

    if (closest_point_light_shadow_caster) {
        // TODO:
        // Ideally we won't be re-allocating everytime we encounter a different sized shadow map. This NEEDS to be
        // solved if we ever allow multiple point light shadow casters
        Vector2ui const required_shadow_resolution =
            get_shadow_quality_resolution(closest_point_light_shadow_caster->shadow_resolution);

        if (!point_light_shadow_cubemap ||
            point_light_shadow_cubemap->get_face_width() != required_shadow_resolution.x ||
            point_light_shadow_cubemap->get_face_height() != required_shadow_resolution.y) {
            reallocate_depth_cubemap(point_light_shadow_cubemap, required_shadow_resolution);
        }
    }
}

void LightSystem::reallocate_depth_target(std::unique_ptr<Framebuffer>& framebuffer, Vector2ui const& new_resolution)
{
    framebuffer = std::make_unique<Framebuffer>(new_resolution, false);
    framebuffer->add_depth_stencil_texture(DepthStencilAttachmentFormat::NormalizedDepthOnly, true)
        .create_framebuffer();
}

void LightSystem::reallocate_depth_cubemap(std::unique_ptr<Cubemap>& cubemap, Vector2ui const& new_resolution)
{
    CubemapSettings settings;
    settings.texture_format = GL_DEPTH_COMPONENT;
    settings.texture_minification_filter_mode = GL_LINEAR;
    settings.texture_magnification_filter_mode = GL_LINEAR;
    cubemap = std::make_unique<Cubemap>(settings);

    cubemap->generate_cubemap_faces(6, GL_TEXTURE_CUBE_MAP_POSITIVE_X, new_resolution, GL_DEPTH_COMPONENT, nullptr);
}

void LightSystem::bind_lightning_uniforms(Shader& shader)
{
    bind_lights(shader, false);
}

void LightSystem::bind_static_lighting_uniforms(Shader& shader)
{
    bind_lights(shader, true);
}

void LightSystem::bind_lights(Shader& shader, bool const bind_only_static)
{
    int num_dir_lights = 0, num_point_lights = 0, num_spot_lights = 0;

    auto group = Scenes::get()
                     ->get_scene()
                     ->get_entity_manager()
                     .get_registry()
                     .group<LightComponent>(entt::get<TransformComponent>);

    for (auto entity : group) {
        auto [transform, light] = group.get<TransformComponent, LightComponent>(entity);

        if (bind_only_static && !light.is_static) {
            continue;
        }

        switch (light.type) {
        case LightType::Directional:
            Log::rt_assert(num_dir_lights < LightBindings::max_dir_lights, "Directional light limit hit");
            LightBindings::bind_directional_light(transform, light, shader, num_dir_lights++);
            break;
        case LightType::Point:
            Log::rt_assert(num_point_lights < LightBindings::max_point_lights, "Point light limit hit");
            LightBindings::bind_point_light(transform, light, shader, num_point_lights++);
            break;
        case LightType::Spot:
            Log::rt_assert(num_spot_lights < LightBindings::max_spot_lights, "Spot light limit hit");
            LightBindings::bind_spot_light(transform, light, shader, num_spot_lights++);
            break;
        case LightType::COUNT:
            break;
        }
    }

    num_dir_lights = std::min<int>(num_dir_lights, LightBindings::max_dir_lights);
    num_point_lights = std::min<int>(num_point_lights, LightBindings::max_point_lights);
    num_spot_lights = std::min<int>(num_spot_lights, LightBindings::max_spot_lights);
    shader.set_uniform("numDirPointSpotLights", Vector4i(num_dir_lights, num_point_lights, num_spot_lights, 0));
}

// Getters
Vector3f LightSystem::get_directional_light_shadow_caster_light_dir() const
{
    if (!closest_directional_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Directional shadow caster does not exist in current scene - could not get light direction"
        );
        return {0.0f, -1.0f, 0.0f};
    }

    return closest_directional_light_shadow_caster_transform->forward();
}

Vector2f LightSystem::get_directional_light_shadow_caster_near_far_plane() const
{
    if (closest_directional_light_shadow_caster == nullptr) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Directional shadow caster does not exist in current scene - could not get near / far plane"
        );
        return {shadowmap_near_plane_default, shadowmap_far_plane_default};
    }

    return {
        closest_directional_light_shadow_caster->shadow_near_plane,
        closest_directional_light_shadow_caster->shadow_far_plane
    };
}

float LightSystem::get_directional_light_shadow_caster_bias() const
{
    if (!closest_directional_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Directional shadow caster does not exist in current scene - could not get bias"
        );
        return shadowmap_bias_default;
    }

    return closest_directional_light_shadow_caster->shadow_bias;
}

int LightSystem::get_directional_light_shadow_caster_index() const
{
    if (!closest_directional_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Directional shadow caster does not exist in current scene - could not get index"
        );
        return -1;
    }

    return closest_directional_light_index;
}

Vector3f LightSystem::get_spot_light_shadow_caster_light_dir() const
{
    if (!closest_spot_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Spotlight shadow caster does not exist in current scene - could not get light direction"
        );
        return {0.0f, -1.0f, 0.0f};
    }

    return closest_spot_light_shadow_caster_transform->forward();
}

Vector3f LightSystem::get_spot_light_shadow_caster_light_position() const
{
    if (!closest_spot_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Spotlight shadow caster does not exist in current scene - could not get light position"
        );
        return {0.0f, 0.0f, 0.0f};
    }

    return closest_spot_light_shadow_caster_transform->position;
}

float LightSystem::get_spot_light_shadow_caster_outer_cut_off_angle() const
{
    if (!closest_spot_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Spotlight shadow caster does not exist in current scene - could not get outer cutoff angle"
        );
        return 0.0f;
    }

    return std::acos(closest_spot_light_shadow_caster->outer_cut_off);
}

float LightSystem::get_spot_light_shadow_caster_attenuation_range() const
{
    if (!closest_spot_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Spotlight shadow caster does not exist in current scene - could not get attenuation range"
        );
        return 0.0f;
    }

    return closest_spot_light_shadow_caster->attenuation_range;
}

Vector2f LightSystem::get_spot_light_shadow_caster_near_far_plane() const
{
    if (!closest_spot_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Spotlight shadow caster does not exist in current scene - could not get near/far plane"
        );
        return {shadowmap_near_plane_default, shadowmap_far_plane_default};
    }

    return {closest_spot_light_shadow_caster->shadow_near_plane, closest_spot_light_shadow_caster->shadow_far_plane};
}

float LightSystem::get_spot_light_shadow_caster_bias() const
{
    if (!closest_spot_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Spotlight shadow caster does not exist in current scene - could not get bias"
        );
        return shadowmap_bias_default;
    }

    return closest_spot_light_shadow_caster->shadow_bias;
}

int LightSystem::get_spot_light_shadow_caster_index() const
{
    if (!closest_spot_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Spotlight shadow caster does not exist in current scene - could not get index"
        );
        return -1;
    }

    return closest_spot_light_index;
}

Vector3f LightSystem::get_point_light_shadow_caster_light_position() const
{
    if (!closest_point_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Point light shadow caster does not exist in current scene - could not get light position"
        );
        return {0.0f, 0.0f, 0.0};
    }

    return closest_point_light_shadow_caster_tranform->position;
}

Vector2f LightSystem::get_point_light_shadow_caster_near_far_plane() const
{
    if (!closest_point_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Point light shadow caster does not exist in current scene - could not get near/far plane"
        );
        return {shadowmap_near_plane_default, shadowmap_far_plane_default};
    }

    return {closest_point_light_shadow_caster->shadow_near_plane, closest_point_light_shadow_caster->shadow_far_plane};
}

float LightSystem::get_point_light_shadow_caster_bias() const
{
    if (!closest_point_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Point light shadow caster does not exist in current scene - could not get bias"
        );
        return shadowmap_bias_default;
    }

    return closest_point_light_shadow_caster->shadow_bias;
}

int LightSystem::get_point_light_shadow_caster_index() const
{
    if (!closest_point_light_shadow_caster) {
        Log::rt_assert(
            false, "Render::LightSystem"
                   "Point light shadow caster does not exist in current scene - could not get index"
        );
        return -1;
    }

    return closest_point_light_index;
}
}