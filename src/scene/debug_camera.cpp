#include "debug_camera.hpp"
#include "scene/components/fps_player.hpp"

#include <engine/engine.hpp>
#include <input/inputs.hpp>
#include <scene/scenes.hpp>
#include <scene/entity.hpp>
#include <math/math.hpp>

namespace xen {
constexpr static Vector3f view_offset(0.0f, 1.8f, 0.0f);

DebugCamera::DebugCamera()
{
    near = 0.1f;
    far = 4098.0f;
    fov = Math::deg_to_rad(70.0f);
}

void DebugCamera::start() {}

void DebugCamera::update()
{
    auto* scene = Scenes::get()->get_scene();
    // auto entity = scene->get_entity_by_component(this);

    auto delta = Engine::get()->get_delta().as_seconds();

    // auto& entity_transform = entity.get_component<TransformComponent>();

    velocity = (transform.position) / delta;
    transform.position = view_offset;

    if (!scene->is_paused()) {
        auto const mouse_x = Inputs::get()->get_axis("mouse_x")->get_amount();
        auto const mouse_y = Inputs::get()->get_axis("mouse_y")->get_amount();
        auto const cursor_hidden = Windows::get()->get_main_window()->is_cursor_hidden();

        Vector2f const rotation_delta = cursor_hidden * Vector2f(mouse_x, mouse_y);

        transform.rotation.y += rotation_delta.x;
        transform.rotation.x += rotation_delta.y;
        transform.rotation.x = std::clamp(transform.rotation.x, Math::deg_to_rad(90.0f), Math::deg_to_rad(270.0f));
    }

    view = Matrix4::view_matrix(transform.position, transform.rotation);
    projection = Matrix4::perspective_matrix(
        get_fov(), Windows::get()->get_main_window()->get_aspect_ratio(), get_near_plane(), get_far_plane()
    );

    view_frustum.update(view, projection);
    view_ray.update(transform.position, {0.5f, 0.5f}, view, projection);
    /*
    auto delta = Engine::get()->get_delta().as_seconds();

    // if (auto scenePlayer = Scenes::get()->get_scene()-><FpsPlayer>()) {
    //     if (auto transformPlayer = scenePlayer->GetEntity()->GetComponent<Transform>()) {
    //         velocity = (transformPlayer->GetPosition() - position) / delta;
    //         position = transformPlayer->GetPosition() + ViewOffset;
    //     }
    // }

    bool const forward = Inputs::get()->get_button("move_forward")->is_down();
    bool const left = Inputs::get()->get_button("move_left")->is_down();
    bool const backward = Inputs::get()->get_button("move_backward")->is_down();
    bool const right = Inputs::get()->get_button("move_right")->is_down();

    velocity.x += static_cast<float>(right - left) * 0.000001f;
    velocity.y += static_cast<float>(backward - forward) * 0.000001f;

    velocity.x = std::max(-5.0f, std::min(5.0f, velocity.x));
    velocity.y = std::max(-5.0f, std::min(5.0f, velocity.y));

    transform.position.x += velocity.x;
    transform.position.y += velocity.y;

    if (!(forward || backward || left || right)) {
        velocity.x *= 0.9f;
        velocity.y *= 0.9f;
    }

    if (!Scenes::get()->get_scene()->is_paused()) {
        auto const mouse_x = Inputs::get()->get_axis("mouse_x")->get_amount();
        auto const mouse_y = Inputs::get()->get_axis("mouse_y")->get_amount();
        auto const cursor_hidden = Windows::get()->get_main_window()->is_cursor_hidden();

        Vector2f const rotation_delta = cursor_hidden * Vector2f(mouse_x, mouse_y);

        transform.rotation.y += rotation_delta.x;
        transform.rotation.x += rotation_delta.y;
        transform.rotation.x = std::clamp(transform.rotation.x, Math::deg_to_rad(90.0f), Math::deg_to_rad(270.0f));
    }

    view = Matrix4::view_matrix(transform.position, transform.rotation);
    projection = Matrix4::perspective_matrix(
        get_fov(), Windows::get()->get_main_window()->get_aspect_ratio(), get_near_plane(), get_far_plane()
    );

    view_frustum.update(view, projection);
    view_ray.update(transform.position, {0.5f, 0.5f}, view, projection);
    */
}
}